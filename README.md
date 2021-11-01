![SnemulDSDS](img/snemulDS-TWL.png)

NTR/TWL SDK: TGDS1.6-Snemulds, specially tailored for SnemulDS TWL mode.

# SnemulDS 0.6c

master: Development branch. Use TGDS1.6-Snemulds: branch for stable features.
SnemulDS 0.6b Original Sources (because codebase it´s not either 0.5 or 0.6a from snemuldsx source codes)... 
This is based from real 0.6.a sources archeide left years ago and I managed to find.

End-users:

Instructions:

How to get the latest working version:

    - Download latest sources from Master branch: https://bitbucket.org/Coto88/SnemulDS/get/TGDS1.6-Snemulds.zip
    - Download and copy, /arm7dldi-ntr -> SnemulDS.nds (NTR mode) or /arm7dldi-twl (TWL mode) -> SnemulDS.srl from /release folder, snemul.cfg as well, in SD:/ root . 
	  If it prompts for overwrite: Yes to All. 
    - Create a /snes folder in SD: root, and put your homebrew games on it
	- SPC Playback: Optionally, create a /spc folder in SD: root, and put your SPC files on it. You can choose and play a SPC File in the "SPC Jukebox" option. 

Hotkeys:
	While SnemulDS runs a game, it is possible to perform the following hotkeys:
	- L + R + START + Right: 	Swap Menu Screen/Emulator framebuffer between Top / Bottom Screen. When framebuffer is set to bottom screen, the top screen will turn off.
	
Remap NDS -> SNES Keys:	
	- Head over to release/snemul.cfg, section: [KEYS] and copy/paste source value into target field, then restore original overwritten value into source field.
	Save changes, then copy snemul.cfg in SD root. Keys are remapped now.

Save / Load States:
	- It is experimental. It may or not work consecutively. It's recommended to always save then restore a single state at a time.


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
