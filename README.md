![SnemulDSDS](img/snemulDS-TWL.png)

NTR/TWL SDK: TGDS1.65

# SnemulDS 0.6c

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
	- It is experimental. It may or not work consecutively. It's recommended to always save then restore a single state at a time.

Properly Saving / Loading SRAM:
	- Save in-game, check Options -> SRAM saving and wait for a "SRAM written" message to appear. 
	Right after uncheck this option before reloading other games or shutting down console. This ensures your savefile is kept safely.

Screen Sync (Vblank Enabled / Vblank Disabled):
	- If the game runs too slow for you, disable vblank. The default and recommended setting is enabled Vblank for most games otherwise these will run too fast becoming unplayable.

Latest stable release: https://bitbucket.org/Coto88/SnemulDS/get/TGDS1.65.zip

Changelog:

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

Thanks to:

Archeide for the emulator & source code
Bubble2k for CPU core
Gladius for Sound Core

Coto.
