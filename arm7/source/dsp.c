#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>//BRK(); SBRK();
#include <fcntl.h>
#include <sys/stat.h>

#include "spcdefs.h"
#include "dsp.h"
#include "apu.h"
#include "ipcfifoTGDSUser.h"

#include "dsp.h"
#include "apu.h"
#include "biosTGDS.h"
#include "main.h"

// Envelope timing table.  Number of counts that should be subtracted from the counter
// The counter starts at 30720 (0x7800).
static const sint16 ENVCNT_START = 0x7800;
static const sint16 ENVCNT[0x20] = {
	0x0000,0x000F,0x0014,0x0018,0x001E,0x0028,0x0030,0x003C,
	0x0050,0x0060,0x0078,0x00A0,0x00C0,0x00F0,0x0140,0x0180,
	0x01E0,0x0280,0x0300,0x03C0,0x0500,0x0600,0x0780,0x0A00,
	0x0C00,0x0F00,0x1400,0x1800,0x1E00,0x2800,0x3C00,0x7800
};

void DspSetEndOfSample(uint32 channel);

struct DspChannel channels[8];
uint8 DSP_MEM[0x100];
sint32 mixBuffer[MIXBUFSIZE * 2];
sint32 echoBuffer[MIXBUFSIZE * 2];
sint16 brrTab[16 * 16];
uint32 firTable[8 * 2 * 2];
uint8 *echoBase;
uint16 dspPreamp ALIGNED = 0x140;
uint16 echoDelay ALIGNED;
uint16 echoCursor ALIGNED;
uint8 firOffset ALIGNED;

// externs from dspmixer.S
uint32 DecodeSampleBlockAsm(uint8 *blockPos, sint16 *samplePos, struct DspChannel *channel);
uint32 DecodeSampleBlockAsmUncached(uint8 *blockPos, sint16 *samplePos, struct DspChannel *channel); //required to remove audio cracks!
extern uint8 channelNum;

uint32 DecodeSampleBlock(struct DspChannel *channel) {
    uint8 *cur = (uint8*)&(APU_MEM[channel->blockPos]);
    sint16 *sample = channel->decoded;

    if (channel->blockPos > 0x10000 - 9) {
        // Set end of block, with no loop
        DspSetEndOfSample(channelNum);
        return 1;
    }

    // Is this the last block?
    if (channel->brrHeader & 1) {
        // channelNum will be set from asm
        DSP_MEM[DSP_ENDX] |= (1 << channelNum);

        if (channel->brrHeader & 2) {
            // Looping
    	    uint8 *sampleBase = APU_MEM + (DSP_MEM[DSP_DIR] << 8);
	        uint16 *lookupPointer = (uint16*)(sampleBase + (DSP_MEM[(channelNum << 4) + DSP_SRC] << 2));
            channel->blockPos = lookupPointer[1];
            cur = (uint8*)&(APU_MEM[channel->blockPos]);
        } else {
            DspSetEndOfSample(channelNum);
            return 1;
        }
    }
    channel->brrHeader = *cur;
    if(apuCacheSamples != 1){
		DecodeSampleBlockAsmUncached(cur, sample, channel);
	}
	else{
		DecodeSampleBlockAsm(cur, sample, channel);
    }
    channel->blockPos += 9;
    return 0;
}

void DspReset() {
    int i = 0, c = 0;
    // Delay for 1 sample
    echoDelay = 4;
    echoCursor = 0;
    echoBase = APU_MEM;

    firOffset = 0;
    for (i = 0; i < MIXBUFSIZE * 4; i++) {
        firTable[i] = 0;
    }

    // Disable echo emulation
    DSP_MEM[DSP_FLAG] = 0x20;

	for (i = 0; i < 8; i++) {
        channels[i].samplePos = 0;
        channels[i].envCount = 0;
        channels[i].active = false;
        channels[i].echoEnabled = false;
	}

    // Build a lookup table for the range values (thanks to trac)
    for (i = 0; i < 13; i++) {
		for (c = 0; c < 16; c++)
			brrTab[(i << 4) + c] = (sint16)((((c ^ 8) - 8) << i) >> 1);
	}
	// range 13-15
    for (i = 13; i < 16; i++) {
		for (c = 0; c < 8; c++)
			brrTab[(i << 4) + c] = 0;
		for(c = 8; c < 16; c++)
			brrTab[(i << 4) + c] = 0xF800;
	}
}

void DspSetChannelVolume(uint32 channel) {
    channels[channel].leftVolume = DSP_MEM[(channel << 4) + DSP_VOL_L];
    channels[channel].rightVolume = DSP_MEM[(channel << 4) + DSP_VOL_R];

    channels[channel].leftCalcVolume = (channels[channel].leftVolume * channels[channel].envx) >> 7;
    channels[channel].rightCalcVolume = (channels[channel].rightVolume * channels[channel].envx) >> 7;
}

void DspSetChannelPitch(uint32 channel) {
	uint16 rawFreq = ((DSP_MEM[(channel << 4) + DSP_PITCH_H] << 8) + DSP_MEM[(channel << 4) + DSP_PITCH_L]) & 0x3fff;

    // Clear low bit of sample speed so we can do a little optimization in dsp mixing
	channels[channel].sampleSpeed = swiDivide(((rawFreq << 3) << 12), MIXRATE) & (~1);
}

void DspSetChannelSource(uint32 channel) {
	uint8 *sampleBase = APU_MEM + (DSP_MEM[DSP_DIR] << 8);
	uint16 *lookupPointer = (uint16*)(sampleBase + (DSP_MEM[(channel << 4) + DSP_SRC] << 2));

    channels[channel].blockPos = lookupPointer[0];
}

void DspSetEndOfSample(uint32 channel) {
    channels[channel].active = false;
    channels[channel].envState = ENVSTATE_NONE;

    channels[channel].envx = 0;
    DSP_MEM[(channel << 4) | DSP_ENVX] = 0;
    DSP_MEM[(channel << 4) | DSP_OUTX] = 0;
}

void DspSetChannelEnvelopeHeight(uint32 channel, uint8 height) {
    channels[channel].envx = height << 8;

    channels[channel].leftCalcVolume = (channels[channel].leftVolume * channels[channel].envx) >> 7;
    channels[channel].rightCalcVolume = (channels[channel].rightVolume * channels[channel].envx) >> 7;
}

void DspStartChannelEnvelope(uint32 channel) {
	uint8 adsr1 = DSP_MEM[(channel << 4) + DSP_ADSR1];

    // ADSR mode, set envelope up
    // Attack rate goes into envelopeSpeed initially
  	uint8 adsr2 = DSP_MEM[(channel << 4) + DSP_ADSR2];
    uint8 decay = (adsr1 >> 4) & 0x7;
    uint8 sustainLevel = adsr2 >> 5;
    uint8 sustainRate = adsr2 & 0x1f;

    channels[channel].decaySpeed = ENVCNT[(decay << 1) + 0x10];
    channels[channel].sustainLevel = 0x10 * (sustainLevel + 1);
    channels[channel].sustainSpeed = ENVCNT[sustainRate];

    // Don't set envelope parameters when we are releasing the note
    if (adsr1 & 0x80) {
        uint8 attack = adsr1 & 0xf;

        if (attack == 0xf) {
            // 0ms attack, go straight to full volume, and set decay
            DspSetChannelEnvelopeHeight(channel, 0x7f);
            channels[channel].envSpeed = channels[channel].decaySpeed;
            channels[channel].envState = ENVSTATE_DECAY;
        } else {
            DspSetChannelEnvelopeHeight(channel, 0);
            channels[channel].envSpeed = ENVCNT[(attack << 1) + 1];
            channels[channel].envState = ENVSTATE_ATTACK;
        }
    } else {
        // Gain mode
    	uint8 gain = DSP_MEM[(channel << 4) + DSP_GAIN];

        if ((gain & 0x80) == 0) {
            // Direct designation
            DspSetChannelEnvelopeHeight(channel, gain & 0x7f);
            channels[channel].envState = ENVSTATE_DIRECT;
        } else {
            DspSetChannelEnvelopeHeight(channel, 0);
            channels[channel].envSpeed = ENVCNT[gain & 0x1f];

            switch ((gain >> 5) & 0x3) {
            case 0:
                // Linear decrease
                channels[channel].envState = ENVSTATE_DECREASE;
                break;
            case 1:
                // Exponential decrease
                channels[channel].envState = ENVSTATE_DECEXP;
                break;
            case 2:
                // Linear increase
                channels[channel].envState = ENVSTATE_INCREASE;
                break;
            case 3:
                // Bent line increase
                channels[channel].envState = ENVSTATE_BENTLINE;
                break;
            }
        }
    }
}

void DspChangeChannelEnvelopeGain(uint32 channel) {
    // Don't set envelope parameters when we are releasing the note
    if (!channels[channel].active) return;
    if (channels[channel].envState == ENVSTATE_RELEASE) return;

    // If in ADSR mode, write to GAIN register has no effect
    if (DSP_MEM[(channel << 4) + DSP_ADSR1] & 0x80) return;

    // Otherwise treat it as GAIN change
	uint8 gain = DSP_MEM[(channel << 4) + DSP_GAIN];

    if ((gain & 0x80) == 0) {
        // Direct designation
        DspSetChannelEnvelopeHeight(channel, gain & 0x7f);
        channels[channel].envState = ENVSTATE_DIRECT;
        channels[channel].envSpeed = 0;
    } else {
        channels[channel].envSpeed = ENVCNT[gain & 0x1f];

        switch ((gain >> 5) & 0x3) {
        case 0:
            // Linear decrease
            channels[channel].envState = ENVSTATE_DECREASE;
            break;
        case 1:
            // Exponential decrease
            channels[channel].envState = ENVSTATE_DECEXP;
            break;
        case 2:
            // Linear increase
            channels[channel].envState = ENVSTATE_INCREASE;
            break;
        case 3:
            // Bent line increase
            channels[channel].envState = ENVSTATE_BENTLINE;
            break;
        }
    }
}

void DspChangeChannelEnvelopeAdsr1(uint32 channel, uint8 orig) {
    // Don't set envelope parameters when we are releasing the note
    if (!channels[channel].active) return;
    if (channels[channel].envState == ENVSTATE_RELEASE) return;

	uint8 adsr1 = DSP_MEM[(channel << 4) + DSP_ADSR1];

    uint8 decay = (adsr1 >> 4) & 0x7;
    channels[channel].decaySpeed = ENVCNT[(decay << 1) + 0x10];

    if (channels[channel].envState == ENVSTATE_ATTACK) {
        uint8 attack = adsr1 & 0xf;
        channels[channel].envSpeed = ENVCNT[(attack << 1) + 1];
    } else if (channels[channel].envState == ENVSTATE_DECAY) {
        channels[channel].envSpeed = channels[channel].decaySpeed;
    }

    if (adsr1 & 0x80) {
        if (!(orig & 0x80)) {
            // Switch to ADSR
            uint8 attack = adsr1 & 0xf;
            channels[channel].envState = ENVSTATE_ATTACK;
            channels[channel].envSpeed = ENVCNT[(attack << 1) + 1];
        }
    } else {
        // Switch to gain mode
        DspChangeChannelEnvelopeGain(channel);
    }
}

void DspChangeChannelEnvelopeAdsr2(uint32 channel) {
    // Don't set envelope parameters when we are releasing the note
    if (!channels[channel].active) return;
    if (channels[channel].envState == ENVSTATE_RELEASE) return;

	uint8 adsr2 = DSP_MEM[(channel << 4) + DSP_ADSR2];
    uint8 sustainRate = adsr2 & 0x1f;
    channels[channel].sustainSpeed = ENVCNT[sustainRate];

    if (channels[channel].envState == ENVSTATE_SUSTAIN) {
        channels[channel].envSpeed = channels[channel].sustainSpeed;
    }
}

void DspKeyOnChannel(uint32 i) {
    channels[i].envState = ENVSTATE_NONE;

    DspSetChannelEnvelopeHeight(i, 0);
    DSP_MEM[(i << 4) | DSP_ENVX] = 0;
    DSP_MEM[(i << 4) | DSP_OUTX] = 0;

    DspSetChannelVolume(i);
    DspSetChannelPitch(i);
    DspSetChannelSource(i);
    DspStartChannelEnvelope(i);

    if(PocketSPCVersion == 10){
    	channels[i].samplePos = 16 << 12;
	}
	else{
		channels[i].samplePos = 0;
	}

    channels[i].brrHeader = 0;
    channels[i].prevSamp1 = 0;
    channels[i].prevSamp2 = 0;

    if(PocketSPCVersion == 9){
    	DecodeSampleBlock(&(channels[i]));
	}

    channels[i].envCount = ENVCNT_START;
    channels[i].active = true;

    if ((DSP_MEM[DSP_NOV]>>i)&1) {
        channels[i].active = false;
        // Noise sample
    }

    DSP_MEM[DSP_ENDX] &= ~(1 << i);
}

void DspPrepareStateAfterReload() {
    int i = 0;
	// Set up echo delay
    DspWriteByte(DSP_MEM[DSP_EDL], DSP_EDL);

    echoBase = APU_MEM + (DSP_MEM[DSP_ESA] << 8);
    for (i = 0; i < echoDelay; i++) {
        echoBase[i] = 0;
    }

	for (i = 0; i < 8; i++) {
        channels[i].echoEnabled = (DSP_MEM[DSP_EON] >> i) & 1;

        if (DSP_MEM[DSP_KON] & (1 << i)) {
            DspKeyOnChannel(i);
        }
	}
}

void DspWriteByte(uint8 val, uint8 address) {
    uint8 orig = DSP_MEM[address];
    DSP_MEM[address] = val;

    if (address > 0x7f) return;

    switch (address & 0xf) {
        case DSP_VOL_L:
			DspSetChannelVolume(address >> 4);
            break;
		case DSP_VOL_R:
			DspSetChannelVolume(address >> 4);
			break;
		case DSP_PITCH_L:
			DspSetChannelPitch(address >> 4);
			break;
		case DSP_PITCH_H:
			DspSetChannelPitch(address >> 4);
			break;
		case DSP_ADSR1:
    		DspChangeChannelEnvelopeAdsr1(address >> 4, orig);
			break;
		case DSP_ADSR2:
			DspChangeChannelEnvelopeAdsr2(address >> 4);
			break;
		case DSP_GAIN:
			DspChangeChannelEnvelopeGain(address >> 4);
			break;

        case 0xC:
            switch (address >> 4) {
            case (DSP_KON >> 4):{
//                val &= ~DSP_MEM[DSP_KOF];
//                DSP_MEM[DSP_KON] = val & DSP_MEM[DSP_KOF];

                if (val) {
					uint32 i=0;
                    DSP_MEM[DSP_KON] = val & DSP_MEM[DSP_KOF];
                    val &= ~DSP_MEM[DSP_KOF];
                    for (i=0; i<8; i++)
                        if ((val>>i)&1) {
                            DspKeyOnChannel(i);
                        }
                }
            }break;

            case (DSP_KOF >> 4):{
                int i=0;
				for (i=0; i<8; i++)
                    if (((val>>i)&1) && channels[i].active && channels[i].envState != ENVSTATE_RELEASE) {
                        // Set current state to release (ENDX will be set when release hits 0)
                        channels[i].envState = ENVSTATE_RELEASE;
                        channels[i].envCount = ENVCNT_START;
                        channels[i].envSpeed = ENVCNT[0x1C];
                    }
    		}break;

            case (DSP_ENDX >> 4):
	    		DSP_MEM[DSP_ENDX] = 0;
		    	break;
            }
            break;

        case 0xD:
            switch (address >> 4) {
            case (DSP_EDL >> 4):
                val &= 0xf;
                if (val == 0) {
                    echoDelay = 4;
                } else {
                    echoDelay = ((uint32)(val << 4) * (MIXRATE / 1000)) << 2;
                }
                break;

            case (DSP_NOV >> 4):{
                int i=0; 
				for (i=0; i<8; i++)
                if ((val>>i)&1) {
                    // TODO: Need to implement noise channels
                    channels[i].active = false;
                }
            }break;

            case (DSP_ESA >> 4):
                echoBase = APU_MEM + (DSP_MEM[DSP_ESA] << 8);
                break;

            case (DSP_EON >> 4):{
                int i = 0;
				for (i = 0; i < 8; i++) {
                    channels[i].echoEnabled = (val >> i) & 1;
                }
				}break;
            }
    }

/*
		case DSP_PMOD:
			for (int i=0; i<8; i++)
				if ((val>>i)&1) {
//					dspunimpl(DSP_PMOD);
//					channels[i].active = 0;
				}
			break;
            */
}

// Gaussian table from Sneese
short gauss[] ALIGNED = {
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 
	0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 0x001, 
	0x001, 0x001, 0x001, 0x002, 0x002, 0x002, 0x002, 0x002, 
	0x002, 0x002, 0x003, 0x003, 0x003, 0x003, 0x003, 0x004, 
	0x004, 0x004, 0x004, 0x004, 0x005, 0x005, 0x005, 0x005, 
	0x006, 0x006, 0x006, 0x006, 0x007, 0x007, 0x007, 0x008, 
	0x008, 0x008, 0x009, 0x009, 0x009, 0x00A, 0x00A, 0x00A, 
	0x00B, 0x00B, 0x00B, 0x00C, 0x00C, 0x00D, 0x00D, 0x00E, 
	0x00E, 0x00F, 0x00F, 0x00F, 0x010, 0x010, 0x011, 0x011, 
	0x012, 0x013, 0x013, 0x014, 0x014, 0x015, 0x015, 0x016, 
	0x017, 0x017, 0x018, 0x018, 0x019, 0x01A, 0x01B, 0x01B, 
	0x01C, 0x01D, 0x01D, 0x01E, 0x01F, 0x020, 0x020, 0x021, 
	0x022, 0x023, 0x024, 0x024, 0x025, 0x026, 0x027, 0x028, 
	0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x02E, 0x02F, 0x030, 
	0x031, 0x032, 0x033, 0x034, 0x035, 0x036, 0x037, 0x038, 
	0x03A, 0x03B, 0x03C, 0x03D, 0x03E, 0x040, 0x041, 0x042, 
	0x043, 0x045, 0x046, 0x047, 0x049, 0x04A, 0x04C, 0x04D, 
	0x04E, 0x050, 0x051, 0x053, 0x054, 0x056, 0x057, 0x059, 
	0x05A, 0x05C, 0x05E, 0x05F, 0x061, 0x063, 0x064, 0x066, 
	0x068, 0x06A, 0x06B, 0x06D, 0x06F, 0x071, 0x073, 0x075, 
	0x076, 0x078, 0x07A, 0x07C, 0x07E, 0x080, 0x082, 0x084, 
	0x086, 0x089, 0x08B, 0x08D, 0x08F, 0x091, 0x093, 0x096, 
	0x098, 0x09A, 0x09C, 0x09F, 0x0A1, 0x0A3, 0x0A6, 0x0A8, 
	0x0AB, 0x0AD, 0x0AF, 0x0B2, 0x0B4, 0x0B7, 0x0BA, 0x0BC, 
	0x0BF, 0x0C1, 0x0C4, 0x0C7, 0x0C9, 0x0CC, 0x0CF, 0x0D2, 
	0x0D4, 0x0D7, 0x0DA, 0x0DD, 0x0E0, 0x0E3, 0x0E6, 0x0E9, 
	0x0EC, 0x0EF, 0x0F2, 0x0F5, 0x0F8, 0x0FB, 0x0FE, 0x101, 
	0x104, 0x107, 0x10B, 0x10E, 0x111, 0x114, 0x118, 0x11B, 
	0x11E, 0x122, 0x125, 0x129, 0x12C, 0x130, 0x133, 0x137, 
	0x13A, 0x13E, 0x141, 0x145, 0x148, 0x14C, 0x150, 0x153, 
	0x157, 0x15B, 0x15F, 0x162, 0x166, 0x16A, 0x16E, 0x172, 
	0x176, 0x17A, 0x17D, 0x181, 0x185, 0x189, 0x18D, 0x191, 
	0x195, 0x19A, 0x19E, 0x1A2, 0x1A6, 0x1AA, 0x1AE, 0x1B2, 
	0x1B7, 0x1BB, 0x1BF, 0x1C3, 0x1C8, 0x1CC, 0x1D0, 0x1D5, 
	0x1D9, 0x1DD, 0x1E2, 0x1E6, 0x1EB, 0x1EF, 0x1F3, 0x1F8, 
	0x1FC, 0x201, 0x205, 0x20A, 0x20F, 0x213, 0x218, 0x21C, 
	0x221, 0x226, 0x22A, 0x22F, 0x233, 0x238, 0x23D, 0x241, 
	0x246, 0x24B, 0x250, 0x254, 0x259, 0x25E, 0x263, 0x267, 
	0x26C, 0x271, 0x276, 0x27B, 0x280, 0x284, 0x289, 0x28E, 
	0x293, 0x298, 0x29D, 0x2A2, 0x2A6, 0x2AB, 0x2B0, 0x2B5, 
	0x2BA, 0x2BF, 0x2C4, 0x2C9, 0x2CE, 0x2D3, 0x2D8, 0x2DC, 
	0x2E1, 0x2E6, 0x2EB, 0x2F0, 0x2F5, 0x2FA, 0x2FF, 0x304, 
	0x309, 0x30E, 0x313, 0x318, 0x31D, 0x322, 0x326, 0x32B, 
	0x330, 0x335, 0x33A, 0x33F, 0x344, 0x349, 0x34E, 0x353, 
	0x357, 0x35C, 0x361, 0x366, 0x36B, 0x370, 0x374, 0x379, 
	0x37E, 0x383, 0x388, 0x38C, 0x391, 0x396, 0x39B, 0x39F, 
	0x3A4, 0x3A9, 0x3AD, 0x3B2, 0x3B7, 0x3BB, 0x3C0, 0x3C5, 
	0x3C9, 0x3CE, 0x3D2, 0x3D7, 0x3DC, 0x3E0, 0x3E5, 0x3E9, 
	0x3ED, 0x3F2, 0x3F6, 0x3FB, 0x3FF, 0x403, 0x408, 0x40C, 
	0x410, 0x415, 0x419, 0x41D, 0x421, 0x425, 0x42A, 0x42E, 
	0x432, 0x436, 0x43A, 0x43E, 0x442, 0x446, 0x44A, 0x44E, 
	0x452, 0x455, 0x459, 0x45D, 0x461, 0x465, 0x468, 0x46C, 
	0x470, 0x473, 0x477, 0x47A, 0x47E, 0x481, 0x485, 0x488, 
	0x48C, 0x48F, 0x492, 0x496, 0x499, 0x49C, 0x49F, 0x4A2, 
	0x4A6, 0x4A9, 0x4AC, 0x4AF, 0x4B2, 0x4B5, 0x4B7, 0x4BA, 
	0x4BD, 0x4C0, 0x4C3, 0x4C5, 0x4C8, 0x4CB, 0x4CD, 0x4D0, 
	0x4D2, 0x4D5, 0x4D7, 0x4D9, 0x4DC, 0x4DE, 0x4E0, 0x4E3, 
	0x4E5, 0x4E7, 0x4E9, 0x4EB, 0x4ED, 0x4EF, 0x4F1, 0x4F3, 
	0x4F5, 0x4F6, 0x4F8, 0x4FA, 0x4FB, 0x4FD, 0x4FF, 0x500, 
	0x502, 0x503, 0x504, 0x506, 0x507, 0x508, 0x50A, 0x50B, 
	0x50C, 0x50D, 0x50E, 0x50F, 0x510, 0x511, 0x511, 0x512, 
	0x513, 0x514, 0x514, 0x515, 0x516, 0x516, 0x517, 0x517, 
	0x517, 0x518, 0x518, 0x518, 0x518, 0x518, 0x519, 0x519
};