//////////////////////////////////////////////////////SnemulDS research//////////////////////////////////////////////////////

There are 4 Input Ouput addressing map layers involved in SnemulDS, order is highest to lowest:

(highest level)

- 1) SnemulDS C core emulated peripheral (PPU or APU core)

- 2) SnemulDS C core middle-layer

- 3) CPU Packed/Unpacked commands: These wrap SNES CPU registers back and forth SnemulDS C core and SnezziDS ARM Core. That process takes place mostly when PPU or SnemulDS IO wants to
read or write to SNES CPU or IO addresses.

(lowest level)
- 4) SnezziDS ARM Core (direct i/o here through 'TranslateAddress' mapping scheme, which takes real SNES IO address and it's translated to direct GBA/NDS IO address.
Then depending if caller node comes from SnemulDS C Core, the whole SNES registers context will be saved, then updated on level 1), then going from from 4) -> 3) -> 2) then 1) until
a new SNES opcode executes. For example (SnezziDS ARM Core) SNES LDB / STB Snes CPU opcodes will resolve whatever SNES page index + page offset into the target NDS SNES allocated buffers, 
added page offset, do either a load or store, then proceed to next SNES opcode.


//////////////////////////////////////////////////////How DSP1 talks to SnemulDS//////////////////////////////////////////////////////
DSP1 RAM, IO (including file registers) is wired through the following methods: 

(DSP1 <-> SNES IO map) 
IO_getbyte 
IO_setbyte 
IO_getword 
IO_setword 

which are glued to 4) (see above). 

-

____Remoteboot____
Also, it's recommended to use the remoteboot feature. It allows to send the current TGDS Project over wifi removing the necessity
to take out the SD card repeteadly and thus, causing it to wear out and to break the SD slot of your unit.

Usage:
- Make sure the wifi settings in the NintendoDS are properly set up, so you're already able to connect to internet from it.

- Get a copy of ToolchainGenericDS-multiboot: http://github.com/cotodevel/ToolchainGenericDS-multiboot/archive/TGDS1.65.zip
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