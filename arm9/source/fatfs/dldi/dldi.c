#include "dsregs.h"
#include "dldi.h"

const u32  DLDI_MAGIC_NUMBER = 
	0xBF8DA5ED;	
	
// Stored backwards to prevent it being picked up by DLDI patchers
const char DLDI_MAGIC_STRING_BACKWARDS [DLDI_MAGIC_STRING_LEN] =
	{'\0', 'm', 'h', 's', 'i', 'h', 'C', ' '} ;

// The only built in driver
extern DLDI_INTERFACE _io_dldi_stub;

const DLDI_INTERFACE* io_dldi_data = &_io_dldi_stub;

const struct DISC_INTERFACE_STRUCT* dldiGetInternal (void) {
	if (_io_dldi_stub.ioInterface.features & FEATURE_SLOT_GBA) {
		setCpuGbaBusAccessPrio(GBASLOT_ARM9BUS);
	}
	if (_io_dldi_stub.ioInterface.features & FEATURE_SLOT_NDS) {
		setCpuNdsBusAccessPrio(NDSSLOT_ARM9BUS);
	}

	return &_io_dldi_stub.ioInterface;
}