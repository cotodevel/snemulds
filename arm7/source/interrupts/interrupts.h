#ifdef __cplusplus
extern "C" {
#endif

extern void IpcSynchandlerUser(uint8 ipcByte);
extern void Timer0handlerUser();
extern void Timer1handlerUser();
extern void Timer2handlerUser();
extern void Timer3handlerUser();
extern void HblankUser();
extern void VblankUser();
extern void VcounterUser();
extern void ScreenlidhandlerUser();

#ifdef __cplusplus
}
#endif
