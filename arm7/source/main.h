#ifndef snemuldsv6arm7_main
#define snemuldsv6arm7_main

typedef void (*type_void)();

#endif


#ifdef __cplusplus
extern "C" {
#endif

// Play buffer, left buffer is first MIXBUFSIZE * 2 u16's, right buffer is next
extern u16 *playBuffer;
extern volatile int soundCursor;
extern int apuMixPosition;
extern int pseudoCnt;
extern int frame;
extern int scanlineCount;
extern u32 interrupts_to_wait_arm7;
extern bool paused;
extern bool SPC_disable;
extern bool SPC_freedom;

extern void SetupSound();
extern void StopSound();
extern void LoadSpc(const u8 *spc);
extern void SaveSpc(u8 *spc);

#ifdef __cplusplus
}
#endif

