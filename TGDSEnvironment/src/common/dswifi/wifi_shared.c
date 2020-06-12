
#include "ipcfifoTGDS.h"
#include "InterruptsARMCores_h.h"
#include "utilsTGDS.h"

#include "timerTGDS.h"

#ifdef ARM7
#include <string.h>
#include "wifi_arm7.h"
#endif

#ifdef ARM9
#include <stdbool.h>

#include "dsregs.h"
#include "dsregs_asm.h"
#include "InterruptsARMCores_h.h"
#include "wifi_arm9.h"

#endif


#include "wifi_shared.h"
//internal DSWIFI code
void DeInitWIFI(){
	#ifdef ARM7
	Wifi_Deinit();
	#endif
	#ifdef ARM9
	Wifi_DisconnectAP();
	Wifi_DisableWifi();
	DisableIrq(IRQ_TIMER3);
	Wifi_SetSyncHandler(NULL);
	if(WifiData != NULL){
		memset(WifiData, 0, sizeof(Wifi_MainStruct));
	}
	sgIP_Hub_RemoveHardwareInterface(wifi_hw);
	TIMERXDATA(3) = 0;
	TIMERXCNT(3) = 0;
	SendFIFOWords(WIFI_DEINIT, 0);
	#endif
}