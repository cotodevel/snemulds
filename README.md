![SnemulDSDS](img/snemulDS-TWL.png)

# SnemulDS 0.6d [NTR/TWL SDK: TGDS1.65]

Stable Release: 

Default game session (No coprocessor) / CX4:
	- https://bitbucket.org/Coto88/SnemulDS/get/TGDS1.65.zip

DSP-1:
	- https://bitbucket.org/Coto88/SnemulDS/get/dsp.zip

S-DD1 (DSi or up only):
	- https://bitbucket.org/Coto88/SnemulDS/get/sdd1.zip

Usage:
    - Download ALL .zip files from [Stable Release] and copy ALL files per .zip file from /release/arm7dldi-[ntr/twl], in SD:/ root. [ntr] mode is for DS mode, and [twl] for DSi mode. 
		(If it prompts for overwrite: Yes to All)
    - Create a /snes folder in SD: root, and put your homebrew games on it.
	- Put the SD card back into the console, turn it on & run ToolchainGenericDS-multiboot[DS(L) is .nds / DSi or up is .twl]. 
	- Press start inside ToolchainGenericDS-multiboot and select SnemulDS[DS(L) is .nds / DSi or up is .twl] from the file browser.
	- Rejoice
	
Gameboy Macro Mode:
	- Option selectable from the touchscreen User Interface. 
	
Properly Saving / Loading SRAM:
	- Save in-game, check Options -> SRAM saving and wait for a "SRAM written" message to appear. 

Screen Sync (Vblank Enabled / Vblank Disabled):
	- By default, Vblank fast is recommended. If the game is running too fast, try Vblank full. Some specific games may need Vblank disabled.
	
TWL Mode:
	-SnemulDS TWL mode has extended memory (16MB), so games like Donkey Kong Country 3 are 100% beatable, but beware, save often as the game tends to crash sometimes.

Megaman X series are 100% beatable on DS/DSLite/DSi/3DS (TGDS1.65 branch):
	[DS Phat, DS Lite & DSi / 3DS]:
		- Megaman X1 on SnemulDS requires "Mega Man X (US) (v1.1)":
		MD5: 8998f2d66a84c1d44ab6387db7314efd
		SHA1: 404f160bbb1a5e9d3e0bfd38edaa448984040899
		SHA256: 152eb370f988b33291d2dddfc48abb42bfa236a08ddc7362e07dd2795e3e37d0
		SHA512: 8b7de3bfa15e192cb1fb5128cba64ef629813a1d3782c7f57dc2f93b0b06a1565aa256f60fa5b550a1241e8e536b18bb27d4e210937d4b4329739e2990136f0b
		
		- Megaman X2 and Megaman X3 are supported by default.
	
	-Megaman X3 Zero Project is compatible on SnemulDS TWL (DSi only) due to lack of RAM.

snemul.cfg:
	It's automatically generated if missing as well kept persistently in the filesystem's SD: root.
	Optionally, you can rebuild it from the "Options" tab, (overwriting the older one) through the button "Config: Reset now" anytime.
	This, because an invalid snemul.cfg may cause known games to not boot anymore, so you can rebuild it anytime without requiring a PC.

#################################################################################################
Changelog:

SnemulDS 0.6d:
	- Added CX4 co processor support! Megaman X2 / Megaman X3 it's entirely playable now. (tweak sprite priority settings if necessary)
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
PeterLemon for CPU tests
#################################################################################################


Coto.