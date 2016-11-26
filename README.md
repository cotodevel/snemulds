# SnemulDS
SnemulDS 0.6 [Revival]

extract all four linker files (linker_files.zip) on this directory:

<path to devkitpro folder>\devkitARM\arm-none-eabi\lib


then create a new folder, copy source code like you would on any DS project
open MSYS and MAKE. 

then copy snemul.cfg and snemulds.nds to root of your SD, bootup cart, and enjoy.

btw, use cycle hacks if you want decent speed.

-changes:

- SPC Playback is using ARM7 DMAs

- Moved from thumb to arm code (-marm)

-re-fixed some arm7 sound code (hopefully fixes allocation problems)

-more cleanup (itcm/dtcm relocated data)

-using nds dma 3 for pixel copy to vram bank

-up to date devkitpro (as of 07/09/2015) , and no more old libfat compiled libs

to do:

-replace fifo commands from libnds to hardware nds fifo ipc

-fix memory leaks when running sfx games




Coto.
