#ifndef __DLDI_H__
#define __DLDI_H__

#include "global_settings.h"
#include "typedefsTGDS.h"
#include "ipcfifoTGDS.h"
#include "biosTGDS.h"
#include "busTGDS.h"
#include <string.h>

#ifdef ARM9
#include "nds_cp15_misc.h"
#endif

#define FEATURE_MEDIUM_CANREAD		0x00000001
#define FEATURE_MEDIUM_CANWRITE		0x00000002
#define FEATURE_SLOT_GBA			0x00000010
#define FEATURE_SLOT_NDS			0x00000020

typedef uint32_t sec_t;

typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(sec_t sector, sec_t numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(sec_t sector, sec_t numSectors, const void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;

enum DldiOffsets {
	DO_magicString = 0x00,			// "\xED\xA5\x8D\xBF Chishm"
	DO_magicToken = 0x00,			// 0xBF8DA5ED
	DO_magicShortString = 0x04,		// " Chishm"
	DO_version = 0x0C,
	DO_driverSize = 0x0D,
	DO_fixSections = 0x0E,
	DO_allocatedSpace = 0x0F,

	DO_friendlyName = 0x10,

	DO_text_start = 0x40,			// Data start
	DO_data_end = 0x44,				// Data end
	DO_glue_start = 0x48,			// Interworking glue start	-- Needs address fixing
	DO_glue_end = 0x4C,				// Interworking glue end
	DO_got_start = 0x50,			// GOT start					-- Needs address fixing
	DO_got_end = 0x54,				// GOT end
	DO_bss_start = 0x58,			// bss start					-- Needs setting to zero
	DO_bss_end = 0x5C,				// bss end

	// IO_INTERFACE data
	DO_ioType = 0x60,
	DO_features = 0x64,
	DO_startup = 0x68,	
	DO_isInserted = 0x6C,	
	DO_readSectors = 0x70,	
	DO_writeSectors = 0x74,
	DO_clearStatus = 0x78,
	DO_shutdown = 0x7C,
	DO_code = 0x80
};

typedef signed int addr_t;
typedef unsigned char data_t;

struct  DISC_INTERFACE_STRUCT{
	unsigned long			ioType ;
	unsigned long			features ;
	FN_MEDIUM_STARTUP		startup ;
	FN_MEDIUM_ISINSERTED	isInserted ;
	FN_MEDIUM_READSECTORS	readSectors ;
	FN_MEDIUM_WRITESECTORS	writeSectors ;
	FN_MEDIUM_CLEARSTATUS	clearStatus ;
	FN_MEDIUM_SHUTDOWN		shutdown ;
} ;

#define LOAD_DEFAULT_NDS 0
#define FIX_ALL						0x01
#define FIX_GLUE					0x02
#define FIX_GOT						0x04
#define FIX_BSS						0x08
#define DEVICE_TYPE_DLDI 0x49444C44

#define DLDI_MAGIC_STRING_LEN 		8
#define DLDI_FRIENDLY_NAME_LEN 		48

// I/O interface with DLDI extensions
struct DLDI_INTERFACE {
	uint32 	magicNumber;
	char	magicString [DLDI_MAGIC_STRING_LEN];
	uint8		versionNumber;
	uint8		driverSize;			// log-2 of driver size in bytes
	uint8		fixSectionsFlags;
	uint8		allocatedSize;		// log-2 of the allocated space in bytes

	char	friendlyName [DLDI_FRIENDLY_NAME_LEN];
	
	// Pointers to sections that need address fixing
	void*	dldiStart;
	void*	dldiEnd;
	void*	interworkStart;
	void*	interworkEnd;
	void*	gotStart;
	void*	gotEnd;
	void*	bssStart;
	void*	bssEnd;
	
	// Original I/O interface data
	struct DISC_INTERFACE_STRUCT ioInterface;
};


extern u32 * DLDIARM7Address;

//ARM7 DLDI implementation
#ifdef ARM7_DLDI
	static inline void setValueSafe(u32 * addr, u32 value){
		#ifdef ARM9
		coherent_user_range_by_size((u32)addr, sizeof(u32));
		#endif
		*addr = value;
	}

	static inline u32 getValueSafe(u32 * addr){
		#ifdef ARM9
		coherent_user_range_by_size((u32)addr, sizeof(u32));
		#endif
		return (u32)*addr;
	}
#endif

static inline addr_t readAddr (data_t *mem, addr_t offset) {
	return ((addr_t*)mem)[offset/sizeof(addr_t)];
}

static inline void writeAddr (data_t *mem, addr_t offset, addr_t value) {
	((addr_t*)mem)[offset/sizeof(addr_t)] = value;
}

static inline addr_t quickFind (const data_t* data, const data_t* search, size_t dataLen, size_t searchLen) {
	const int* dataChunk = (const int*) data;
	int searchChunk = ((const int*)search)[0];
	addr_t i;
	addr_t dataChunkEnd = (addr_t)(dataLen / sizeof(int));

	for ( i = 0; i < dataChunkEnd; i++) {
		if (dataChunk[i] == searchChunk) {
			if ((i*sizeof(int) + searchLen) > dataLen) {
				return -1;
			}
			if (memcmp (&data[i*sizeof(int)], search, searchLen) == 0) {
				return i*sizeof(int);
			}
		}
	}
	return -1;
}

#ifdef ARM9
	extern struct DLDI_INTERFACE _dldi_start;

	//ARM7 DLDI implementation
	#ifdef ARM7_DLDI
	#define DLDI_CLUSTER_SIZE_BYTES	(int)(64*512)
	extern u8 * ARM7DLDIBuf;
	#endif
#endif

static inline struct DLDI_INTERFACE* dldiGet(void) {
	struct DLDI_INTERFACE * dldiInterface = NULL;
	//ARM7 DLDI implementation
	#ifdef ARM7_DLDI
		#ifdef ARM7
		dldiInterface = (struct DLDI_INTERFACE *)DLDIARM7Address;
		if (dldiInterface->ioInterface.features & FEATURE_SLOT_GBA) {
			SetBusSLOT1ARM9SLOT2ARM7();
		}
		if (dldiInterface->ioInterface.features & FEATURE_SLOT_NDS) {
			SetBusSLOT1ARM7SLOT2ARM9();
		}
		#endif
		#ifdef ARM9
		//Just get the DLDI data. No DLDI init code at all.
		dldiInterface = (struct DLDI_INTERFACE*)&_dldi_start;
		#endif
	#endif
	#ifdef ARM9_DLDI
		//No DLDI data from ARM7.
		#ifdef ARM7
		#endif
		#ifdef ARM9
		dldiInterface = (struct DLDI_INTERFACE*)&_dldi_start;
		if (dldiInterface->ioInterface.features & FEATURE_SLOT_GBA) {
			SetBusSLOT1ARM7SLOT2ARM9();
		}
		if (dldiInterface->ioInterface.features & FEATURE_SLOT_NDS) {
			SetBusSLOT1ARM9SLOT2ARM7();
		}
		#endif
	#endif
	return (struct DLDI_INTERFACE*)dldiInterface;
}

static inline bool dldi_handler_read_sectors(sec_t sector, sec_t numSectors, void* buffer){
	//ARM7 DLDI implementation
	#ifdef ARM7_DLDI
		#ifdef ARM7
		struct  DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)DLDIARM7Address;
		return dldiInterface->ioInterface.readSectors(sector, numSectors, buffer);
		#endif
		#ifdef ARM9
		void * targetMem = (void *)((int)ARM7DLDIBuf + 0x400000);
		
		uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
		fifomsg[0] = (uint32)sector;
		fifomsg[1] = (uint32)numSectors;
		fifomsg[2] = (uint32)targetMem;
		fifomsg[7] = (uint32)TGDS_DLDI_ARM7_READ;
		sendByteIPCIndirect(IPC_SERVE_DLDI7_REQBYIRQ);sendIPCIRQOnly();
		while(fifomsg[7] == TGDS_DLDI_ARM7_READ){
			swiDelay(2);
		}
		memcpy((uint16_t*)buffer, (uint16_t*)targetMem, (numSectors * 512));
		#endif	
	#endif	
	#ifdef ARM9_DLDI
		#ifdef ARM7
		return false;
		#endif
		#ifdef ARM9
		struct  DLDI_INTERFACE* dldiInterface = (struct  DLDI_INTERFACE*)DLDIARM7Address;
		return dldiInterface->ioInterface.readSectors(sector, numSectors, buffer);
		#endif
	#endif
}

static inline bool dldi_handler_write_sectors(sec_t sector, sec_t numSectors, const void* buffer){
	//ARM7 DLDI implementation
	#ifdef ARM7_DLDI
		#ifdef ARM7
		struct  DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)DLDIARM7Address;
		return dldiInterface->ioInterface.writeSectors(sector, numSectors, buffer);
		#endif
		#ifdef ARM9
		void * targetMem = (void *)((int)ARM7DLDIBuf + 0x400000);
		memcpy((uint16_t*)targetMem, (uint16_t*)buffer, (numSectors * 512));
		
		uint32 * fifomsg = (uint32 *)&TGDSIPC->fifoMesaggingQueue[0];
		fifomsg[3] = (uint32)sector;
		fifomsg[4] = (uint32)numSectors;
		fifomsg[5] = (uint32)targetMem;
		fifomsg[8] = (uint32)TGDS_DLDI_ARM7_WRITE;
		sendByteIPCIndirect(IPC_SERVE_DLDI7_REQBYIRQ);sendIPCIRQOnly();
		while(fifomsg[8] == TGDS_DLDI_ARM7_WRITE){
			swiDelay(2);
		}
		#endif	
	#endif	
	#ifdef ARM9_DLDI
		#ifdef ARM7
		return false;
		#endif
		#ifdef ARM9
		struct  DLDI_INTERFACE* dldiInterface = (struct  DLDI_INTERFACE*)DLDIARM7Address;
		return dldiInterface->ioInterface.writeSectors(sector, numSectors, buffer);
		#endif
	#endif
}

static inline bool getisTGDSARM7DLDIEnabled(){
	
	#ifdef ARM9
	coherent_user_range_by_size((u32)&TGDSIPC->ARM7DldiEnabled, sizeof(TGDSIPC->ARM7DldiEnabled));
	#endif
	return (bool)TGDSIPC->ARM7DldiEnabled;
}

static inline void setisTGDSARM7DLDIEnabled(bool value){
	
	#ifdef ARM9
	coherent_user_range_by_size((u32)&TGDSIPC->ARM7DldiEnabled, sizeof(TGDSIPC->ARM7DldiEnabled));
	#endif
	TGDSIPC->ARM7DldiEnabled = value;
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern bool dldi_handler_init();
extern void dldi_handler_deinit();

//ARM7DLDI:
//ARM7: ARM7 physical DLDI target location address
//ARM9: Uses this as a pointer to: ARM7 physical DLDI target location address

//ARM9DLDI:
//ARM7: NULL ptr
//ARM9: Global Physical DLDI section (rather than &_dldi_start, since it's discarded at TGDS init)
extern u32 * getDLDIARM7Address();				//	/
extern void setDLDIARM7Address(u32 * address);	//	| Must be defined/standardized by the TGDS project at runtime. This way we ensure IWRAM 64K compatibility + DLDI at ARM7
extern void initDLDIARM7(u32 srcDLDIAddr);		//	/


//ARM7 DLDI implementation
#ifdef ARM7_DLDI
	#ifdef ARM9
	extern void ARM7DLDIInit(u32 targetDLDI7Address);
	extern void ARM9DeinitDLDI();
	#endif
#endif

#ifdef ARM7
extern void ARM7DLDIInit();
#endif

//DldiRelocatedAddress == target DLDI relocated address
//dldiSourceInRam == physical DLDI section having a proper DLDI driver used as donor 
//dldiOutWriteAddress == new physical DLDI out buffer, except, relocated to a new DldiRelocatedAddress!
extern bool dldiRelocateLoader(bool clearBSS, u32 DldiRelocatedAddress, u32 dldiSourceInRam, u32 dldiOutWriteAddress);
//original DLDI code: seeks a DLDI section in binData, and uses current NTR TGDS homebrew's DLDI to relocate it in there
extern bool dldiPatchLoader(data_t *binData, u32 binSize, u32 physDLDIAddress); 

#ifdef __cplusplus
}
#endif
