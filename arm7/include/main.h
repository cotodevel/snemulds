#ifndef snemuldsv6arm7_main
#define snemuldsv6arm7_main

typedef void (*type_void)();

#endif


#ifdef __cplusplus
extern "C" {
#endif

// Play buffer, left buffer is first MIXBUFSIZE * 2 uint16's, right buffer is next
extern uint16 *playBuffer;
extern volatile int soundCursor;
extern int apuMixPosition;
extern int pseudoCnt;
extern int frame;
extern int scanlineCount;
extern uint32 interrupts_to_wait_arm7;
extern bool paused;
extern bool SPC_disable;
extern bool SPC_freedom;

extern void SetupSoundSnemulDS();
extern void StopSoundSnemulDS();
extern void LoadSpc(const uint8 *spc);
extern void SaveSpc(uint8 *spc);

#ifdef __cplusplus
}
#endif

