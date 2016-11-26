#ifdef __cplusplus
extern "C"{
#endif

extern u32 buf_ipc;

extern void HandleFifo();

extern void sendbyte_ipc(uint8 word);
extern u8 recvbyte_ipc();

#ifdef __cplusplus
}
#endif
