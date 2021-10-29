# SnemulDS

Branch: TGDS1.6-Snemulds-Singles. This branch allows to embed a ROM file inside the emulator, whose file sizes can be up to 1.5MB. Made for DownloadPlay where no DLDI exists. 

SnemulDS as of 16 December 2020 is no longer supported by me, Coto. So I'm keeping this branch (along the same branch name in ToolchainGenericDS repository),
if somebody else wants to continue development. This last checkpoint is known to work with SnemulDS.

SnemulDS 0.6b Original Sources (because codebase it´s not either 0.5 or 0.6a from snemuldsx source codes)... 
This is based from real 0.6.a sources archeide left years ago and I managed to find.

End-users:

Instructions:

How to get the latest working version:

    - Download latest sources from Master branch: https://bitbucket.org/Coto88/SnemulDS/get/HEAD.zip
    - Download and copy, either /arm7dldi -> SnemulDS.nds or /arm9dldi -> SnemulDS.nds from /release folder, snemul.cfg as well, in SD:/ root . If it prompts for overwrite: Yes to All. 
	  arm7dldi and/or arm9dldi may work in your card. The one working will emit emulation sound.
    - Create a /snes folder in SD: root, and put your homebrew games on it
	- SPC Playback: Optionally, create a /spc folder in SD: root, and put your SPC files on it. You can choose and play a SPC File in the "SPC Jukebox" option. 
	- Then run SNEmulDS.nds

Hotkeys:

	While SnemulDS runs a game, it is possible to perform the following hotkeys:
	- L + R + START + Right: 	Swap Menu Screen/Emulator framebuffer between Top / Bottom Screen. When framebuffer is set to bottom screen, the top screen will turn off.
	

Notes:
/release folder has precompiled binaries for you to enjoy


Compile Toolchain: To compile this project you will need to follow the steps at https://bitbucket.org/Coto88/toolchaingenericds : Then simply extract the project somewhere.

Compile this project: 

[Windows]
Open msys, through msys commands head to the directory you extracted this project. 
Then write:
 
>make clean
>make

[Linux]
Then write:
 
>make clean
>make

After compiling, run the example in NDS.

Project Specific description: 

Coto: added ZIP support:
The code was taken from NesDS, and changed some stuff so it could work with Toolchain Generic DS.

Requisites:

a) Every .zip file must be within root:/snes/ folder, where root is your sd letter.
b) The file extension must be renamed to .smc before compression.
c) The compressed file must use  .zip (deflate) or .gz, and the compressed .zip must have 1 only file.


[Optional]

GDB Remote debugging: 
To enable GDB Remote Debugging you need: common/ipcfifoTGDSUser.h and remove the trailing "//" (without commas):
//#define GDB_ENABLE

1.  Then recompile the project. Run SnemulDS and games won´t boot while GDB Remote debugging is running, this is normal. 
2.  While 1) takes place, follow the steps @ https://bitbucket.org/Coto88/toolchaingenericds-gdbstub-example

To disable GDB Remote debugging add the trailing "//" you edited in Optional) step, and recompile the project.


EmbeddedFile mode:
1) To build a SnemulDS.nds that embeds & runs directly a binary file, such file must be renamed into "fileEmbed.bin" (no quotes)
then copied over onto arm9/data/ folder. 

2) Head over to ipcfifoTGDSUser.h and define the SNEMULDS_EMBEDDED_FILE value (if it had a leading "//", remove them)

3) You will need to follow the [Compile Toolchain] steps to build such .NDS binary.

Note: It follows the standard NDS NTR format. Any file equal or below 1.5MB will work. 

Remap NDS -> SNES Keys:
Head over to release/snemul.cfg, section: [KEYS] and copy/paste source value into target field, then restore original overwritten value into source field.
Save changes, then copy snemul.cfg in SD root. Keys are remapped now.

Save / Load States:

It is experimental. It may or not work consecutively. It's recommended to always save then restore a single state at a time.

To do:

-   fix Mode 0 (no transparency/broken)
-   fix memory leaks when games exceed the existing paging memory pool (6MB games)


thanks to:

Archeide for the emulator & source code
Bubble2k for CPU core
Gladius for Sound Core


Coto.
