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


//////////////////////////////////////////////////////How CX4 talks to SnemulDS//////////////////////////////////////////////////////
CX4 RAM, IO (including file registers, data and program bank, etc) is wired through the following methods: 

(CX4 <-> SNES IO map) 
IO_getbyte 
IO_setbyte 
IO_getword 
IO_setword 

which are glued to 4) (see above). 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Todo SnemulDS on VS2012:
Use Data Bank Register to reload a Page and Program Bank Register to reload a Page while in Snes9X, remove original Snes9X Snes IO mapper and use the SnemulDS one.
