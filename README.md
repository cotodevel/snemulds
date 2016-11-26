# SnemulDS
SnemulDS 0.6 [Revival]

extract all four linker files (linker_files.zip) on this directory:

<path to devkitpro folder>\devkitARM\arm-none-eabi\lib


then create a new folder, copy source code like you would on any DS project
open MSYS and MAKE. 

then copy snemul.cfg and snemulds.nds to root of your SD, bootup cart, and enjoy.

btw, use cycle hacks if you want decent speed.


Coto, changes:

-   re-fixed some arm7 sound code, rewritten interrupts on both ARM Cores, includes APU     (IRQs 100% hardware now)
    this means less battery usage, and better use of NDS hardware.

-   some code cleanup, fixed 0x06000000 vram conflicts for APU / background engines mapping (bg engines now use 0x06020000)

-   using nds dma 3 for pixel copy to vram bank

-   up to date devkitpro , using libfat sources directly instead a precompiled library.

-   replaced fifo commands from libnds. Now using a new NDS hardware FIFO overlayer (written by Coto), which fixes
    most games that didn't boot on earlier builds, (but they did boot on old Snemuldsv6 0.2 precompiled from SnemulDS site).

-   new IPC FIFO hardware API that allows to read/write from map that is ONLY available to the other ARM Core.
    Ie: You want to read/write 0x037f8000 which is ARM7-only, now you can from ARM9! with VERY little overhead due to hardware IPC.
    This has been tested and works, but its currently unused.
    
to do:

-   fix Mode 0 (no transparency/broken)
-   fix memory leaks when games exceed the existing paging memory pool (6MB games)


thanks to:

Archeide for the emulator & source code
Bubble2k for CPU core
Gladius for Sound Core


Coto.
