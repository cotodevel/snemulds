#ifndef __dsp_h__
#define __dsp_h__

#include "typedefsTGDS.h"

struct _DspChannel {
    int sampleSpeed;
    int samplePos;
    int envCount;
    int envSpeed;
    sint16 prevDecode[4];
    sint16 decoded[16];
    uint16 decaySpeed;
    uint16 sustainSpeed;
    sint16 envx;
    sint16 prevSamp1, prevSamp2;
    uint16 blockPos;
    sint16 leftCalcVolume;
    sint16 rightCalcVolume;
    uint8 envState;
    uint8 sustainLevel;
    sint8 leftVolume;
    sint8 rightVolume;
    sint8 keyWait;
    bool active;
    uint8 brrHeader;
    bool echoEnabled;
} ALIGNED;
typedef struct _DspChannel DspChannel;

// DSP Register defintions

// Channel specific
#define DSP_VOL_L		0x00
#define DSP_VOL_R		0x01
#define DSP_PITCH_L		0x02
#define DSP_PITCH_H		0x03
#define DSP_SRC			0x04
#define DSP_ADSR1		0x05
#define DSP_ADSR2		0x06
#define DSP_GAIN		0x07
#define DSP_ENVX		0x08
#define DSP_OUTX		0x09

// Global
#define DSP_MAINVOL_L	0x0C
#define DSP_MAINVOL_R	0x1C
#define DSP_ECHOVOL_L	0x2C
#define DSP_ECHOVOL_R	0x3C
#define DSP_KON			0x4C
#define DSP_KOF			0x5C
#define DSP_FLAG		0x6C
#define DSP_ENDX		0x7C

#define DSP_EFB			0x0D
#define DSP_PMOD		0x2D
#define DSP_NOV			0x3D
#define DSP_EON			0x4D
#define DSP_DIR			0x5D
#define DSP_ESA			0x6D
#define DSP_EDL			0x7D

// Envelope state definitions
#define ENVSTATE_NONE       0
#define ENVSTATE_ATTACK		1
#define ENVSTATE_DECAY		2
#define ENVSTATE_SUSTAIN	3
#define ENVSTATE_RELEASE	4
#define ENVSTATE_DIRECT		5
#define ENVSTATE_INCREASE	6
#define ENVSTATE_BENTLINE	7
#define ENVSTATE_DECREASE	8
#define ENVSTATE_DECEXP		9

#endif


#ifdef __cplusplus
extern "C" {
#endif

extern uint8 DSP_MEM[0x100];
extern uint16 dspPreamp;
// externs from dspmixer.S
extern uint32 DecodeSampleBlockAsm(uint8 *blockPos, sint16 *samplePos, DspChannel *channel);
extern uint8 channelNum;
extern void DspMixSamplesStereo(uint32 samples, uint16 *mixBuf);
extern void DspWriteByte(uint8 val, uint8 address);
extern void DspSetEndOfSample(uint32 channel);
extern DspChannel channels[8];
extern void DspReset();
extern void DspPrepareStateAfterReload();
extern uint32 DecodeSampleBlock(DspChannel *channel);

#ifdef __cplusplus
}
#endif
