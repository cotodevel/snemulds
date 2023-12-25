![SnemulDSDS](img/snemulDS-TWL.png)

NTR/TWL SDK: TGDS1.65

# SnemulDS 0.6d

master: Development branch. Use TGDS1.65: branch for stable features.
SnemulDS 0.6b Original Sources (because codebase it´s not either 0.5 or 0.6a from snemuldsx source codes)... 
This is based from real 0.6.a sources archeide left years ago and I managed to find.

Usage:
    - Download and copy all files from /release/arm7dldi-[ntr/twl], snemul.cfg as well, in SD:/ root. [NDS] mode is for DS mode, and [TWL] for DSi mode. 
	  If it prompts for overwrite: Yes to All. 
    - Create a /snes folder in SD: root, and put your homebrew games on it
	- SPC Playback: Optionally, create a /spc folder in SD: root, and put your SPC files on it. You can choose and play a SPC File in the "SPC Jukebox" option. 
	- [NTR]: Now open loader (internal, hbmenu or other), and run ToolchainGenericDS-multiboot.nds. Then select SnemulDS.nds from the menu, choose ARG (A button) and select the snes file to run.
	- [TWL]: Now open TWiLightMenu (you must set it up first, so you can run TWL mode apps), and run ToolchainGenericDS-multiboot.srl. Then select SnemulDS.srl from the menu, choose ARG (A button) and select the snes file to run.
Hotkeys:
	While SnemulDS runs a game, it is possible to perform the following hotkeys:
	- L + R + START + Right: 	Swap Menu Screen/Emulator framebuffer between Top / Bottom Screen. When framebuffer is set to bottom screen, the top screen will turn off.
	
Remap NDS -> SNES Keys:	
	- Head over to release/snemul.cfg, section: [KEYS] and copy/paste source value into target field, then restore original overwritten value into source field.
	Save changes, then copy snemul.cfg in SD root. Keys are remapped now.

Save / Load States:
	- It is experimental. It may or not work. It's recommended to always save then restore a single state at a time.

Properly Saving / Loading SRAM:
	- Save in-game, check Options -> SRAM saving and wait for a "SRAM written" message to appear. 

Screen Sync (Vblank Enabled / Vblank Disabled):
	- If the game is running too fast, enable Vblank. Some specific games will need Vblank disabled.
	Vblank fast forces the emulator to draw a whole frame, preventing frameskips, but won't slow down to sync to NDS Vblank interrupts. (fixes TWL mode to match NTR mode rendering speed)
	Vblank full forces the emulator + NDS Vblank interrupt to draw a whole frame, preventing frameskips. Cause massive slowdown, even more in TWL mode due to ARM9i Vblank bug, but required for some games.
	
	Recommended setting is Vblank fast to get a smooth framerate while fast rendering.
	
TWL Mode:
	-SnemulDS TWL mode has extended memory (16MB), so hacks like Megaman X3 Zero Project are supported and known to work. (TGDS1.65 branch)
	-Donkey Kong Country 3 is playable until the end using SnemulDS TWL mode, but beware, save often as the game tends to crash sometimes.

____Remoteboot____
Also, it's recommended to use the remoteboot feature. It allows to send the current TGDS Project over wifi removing the necessity
to take out the SD card repeteadly and thus, causing it to wear out and to break the SD slot of your unit.

Usage:
- Make sure the wifi settings in the NintendoDS are properly set up, so you're already able to connect to internet from it.

- Get a copy of ToolchainGenericDS-multiboot: https://bitbucket.org/Coto88/ToolchainGenericDS-multiboot/get/TGDS1.65.zip
Follow the instructions there and get either the TWL or NTR version. Make sure you update the computer IP address used to build TGDS Projects, 
in the file: toolchaingenericds-multiboot-config.txt of said repository before moving it into SD card.

For example if you're running NTR mode (say, a DS Lite), you'll need ToolchainGenericDS-multiboot.nds, tgds_multiboot_payload_ntr.bin
and toolchaingenericds-multiboot-config.txt (update here, the computer's IP you use to build TGDS Projects) then move all of them to root SD card directory.

- Build the TGDS Project as you'd normally would, and run these commands from the shell.
<make clean>
<make>

- Then if you're on NTR mode:
<remoteboot ntr_mode computer_ip_address>

- Or if you're on TWL mode:
<remoteboot twl_mode computer_ip_address>

- And finally boot ToolchainGenericDS-multiboot, and press (X), wait a few seconds and TGDS Project should boot remotely.
  After that, everytime you want to remoteboot a TGDS Project, repeat the last 2 steps. ;-)




Latest stable release: https://bitbucket.org/Coto88/SnemulDS/get/sdd1.zip

Changelog:

SnemulDS 0.6d:
	- Added S-DD1 co processor support! Street Fighter Alpha 2 is fully playable now. Star Ocean is UNSUPPORTED. TWL mode ONLY. Because the NTR CPU is too slow to handle this co processor. (tweak sprite priority settings if necessary)
	- fixed even more CPU bugs. This narrows down games not booting due to cross-boundary pages or to APU synchronization issues!
	

SnemulDS 0.6c:
	- fix TWL mode touchscreen!
	
SnemulDS 0.6c alpha:
	- TWL support! Currently tested on no&gba. Some lag may happen between loading screens. 
	- Fixed a ton of issues (a work of at least 3 years, related with TGDS SDK, mostly memory issues, codebase stability, interrupts, arm7 dldi support, arm7 sd twl support, etc)
	- Fixed codebase mostly to have a better user-experience, and of course all the changes were merged into NTR version as well.
	
SnemulDS 0.6b:
	- Fix compatibility with more cards, also restored compatibility to be the same as the one developed by Archeide.

SnemulDS 0.6a:
	- A ton of stuff. Add SnemulDS to TGDS. NTR mode only.

SnemulDS 0.6:
	- used old SnemulDSx sources, which were broken.

Known issues:
	-   Fix Mode 0 (no transparency/broken)

Many thanks to:

Archeide for the emulator & source code
Bubble2k for CPU core
Gladius for Sound Core

PeterLemon: https://github.com/PeterLemon/SNES for allowing to fix some CPU bugs

#################################################################################################

Snemulds 2022 Core Tests:

PlotLineMode7.sfc -> Pass
PlotPixelMode7.sfc -> Pass

8x8BGMap8BPP32x32.sfc -> Pass
8x8BGMap8BPP32x64.sfc -> Pass
8x8BGMap8BPP64x32.sfc -> Pass
8x8BGMap8BPP64x64.sfc -> Pass
8x8BGMapTileFlip.sfc -> Pass

BANKWRAM.sfc -> Pass
BANKLoROMSlowROM.sfc -> Pass
BANKLoROMFastROM.sfc -> Pass
BANKHiROMSlowROM.sfc -> Pass
BANKHiROMFastROM.sfc -> Pass

BANKWRAM.sfc -> Pass


CPU Opcodes implemented OK:
CPUADC.sfc -> Pass
CPUAND.sfc -> Pass
CPUASL.sfc -> Pass
CPUBIT.sfc -> Pass
CPUBRA.sfc -> Pass
CPUCMP.sfc -> Pass
CPUDEC.sfc -> Pass
CPUEOR.sfc -> Pass
CPUINC.sfc -> Pass
CPUJMP.sfc -> Pass
CPULDR.sfc -> Pass
CPULSR.sfc -> Pass
CPUMSC.sfc -> Pass
CPUORA.sfc -> Pass
CPUPSR.sfc -> Pass
CPURET.sfc -> Pass
CPUROL.sfc -> Pass
CPUROR.sfc -> Pass
CPUSTR.sfc -> Pass
CPUSBC.sfc -> Pass
CPUTRN.sfc -> Pass (Emulation mode may require some tweaking)
CPUPHL.sfc -> Pass
CPUMOV.sfc -> Pass


(SNES) CPU Opcodes Failing:
None! 

S-DD1 coprocessor and the rest of non-coprocessor games should be playable now. 
There are still bugs on the cross-boundary pages, which means the SnemulDS ROM mapper on over 4M games is broken and requires fixing.
If the NDS had enough memory yeah, all games would have worked right away, but we'll see what can be done about it ;-)


Note: Other coprocessor opcodes aren't implemented.


----

SnemulDS APU Tests:

SPC700DEC.sfc -> Pass
SPC700EOR.sfc -> Pass
SPC700INC.sfc -> Pass
SPC700ORA.sfc -> Pass
SPC700AND.sfc -> Pass
SPC700SBC.sfc -> Pass
SPC700ADC.sfc -> Pass

--
Snes9X debugger should be:

opcodes: (*CPU.PC++;)

120 (Registers.PC: 0x8000)
24  (Registers.PC: 0x8000)
24  (Registers.PC: 0x8000)
156 (Registers.PC: 0x8000)
156 (Registers.PC: 0x8000)
156 (Registers.PC: 0x8000)
169 (Registers.PC: 0x8000)
143 (Registers.PC: 0x8000)
34 (Registers.PC: 0x8000)
156 (Registers.PC: 0x8000)
169 (Registers.PC: 0x8000)
141 (Registers.PC: 0x8000)
169 (Registers.PC: 0x8000)
141 (Registers.PC: 0x8000)

Coto.