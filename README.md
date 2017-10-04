# SnemulDS
SnemulDS 0.6b [Revival]


10/03/2017

Coto: SnemulDS 0.6b - added ZIP support:
	The code was taken from NesDS, and changed some stuff so it could work with Toolchain Generic DS.
	The requisites are:
	a) Every .zip file must be within root:/snes/ folder, where root is your sd letter
	b) The file extension must be renamed to .smc before compression
	c) The compressed file must use  .zip (deflate) or .gz, and the compressed .zip must have 1 only file




1)
You will need the NintendoDS toolchain "toolchain generic" to build these sources.
Follow the steps at: https://github.com/cotodevel/ToolchainGenericDS to set up the Nintendo DS "toolchain generic" toolchain, GNU licensed. 

2)
now run "Make" inside SnemulDS folder, project should compile fine.

3)
After building simply copy snemul.cfg and snemulds.nds to root of your SD, and enjoy.

    
to do:

-   fix Mode 0 (no transparency/broken)
-   fix memory leaks when games exceed the existing paging memory pool (6MB games)


thanks to:

Archeide for the emulator & source code
Bubble2k for CPU core
Gladius for Sound Core


Coto.