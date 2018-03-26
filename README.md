# SnemulDS
SnemulDS 0.6 Original Sources (because codebase it´s not either 0.5 or 0.6a from snemuldsx source codes).

21/10/2017

After a lot of work (rewrite a toolchain, create a proper interrupt friendly environment required for ToolchainGenericDS and emulators, dswifi compatible)
This is the original SnemulDS 0.6 sources ported to ToolchainGenericDS. It is not based on SnemulDS revival.


- You will need the NintendoDS toolchain "toolchain generic" to build these sources.
Follow the steps at: https://github.com/cotodevel/ToolchainGenericDS to set up the Nintendo DS "toolchain generic" toolchain, GNU licensed. 

- use the d7c8989 ToolchainGenericDS branch from master to compile this, any other commit will not compile on these sources.

- Now run "Make" inside SnemulDS folder, project should compile fine.

- After building simply copy snemul.cfg and snemulds.nds to root of your SD, and enjoy.

    
to do:

-   fix Mode 0 (no transparency/broken)
-   fix memory leaks when games exceed the existing paging memory pool (6MB games)


thanks to:

Archeide for the emulator & source code
Bubble2k for CPU core
Gladius for Sound Core


Coto.