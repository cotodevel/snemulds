//Protection Unit DCACHE / ICACHE setup
#define PU_PAGE_4K		(0x0B << 1)
#define PU_PAGE_8K		(0x0C << 1)
#define PU_PAGE_16K		(0x0D << 1)
#define PU_PAGE_32K		(0x0E << 1)
#define PU_PAGE_64K		(0x0F << 1)
#define PU_PAGE_128K		(0x10 << 1)
#define PU_PAGE_256K		(0x11 << 1)
#define PU_PAGE_512K		(0x12 << 1)
#define PU_PAGE_1M		(0x13 << 1)
#define PU_PAGE_2M		(0x14 << 1)
#define PU_PAGE_4M		(0x15 << 1)
#define PU_PAGE_8M		(0x16 << 1)
#define PU_PAGE_16M		(0x17 << 1)
#define PU_PAGE_32M		(0x18 << 1)
#define PU_PAGE_64M		(0x19 << 1)
#define PU_PAGE_128M		(0x1A << 1)
#define PU_PAGE_256M		(0x1B << 1)
#define PU_PAGE_512M		(0x1C << 1)
#define PU_PAGE_1G		(0x1D << 1)
#define PU_PAGE_2G		(0x1E << 1)
#define PU_PAGE_4G		(0x1F << 1)

#ifdef __cplusplus
extern "C" {
#endif

extern void pu_Enable();
extern void pu_SetRegion(u32 region, u32 value);

extern void pu_SetDataPermissions(u32 v);
extern void pu_SetCodePermissions(u32 v);
extern void pu_SetDataCachability(u32 v);
extern void pu_SetCodeCachability(u32 v);
extern void pu_GetWriteBufferability(u32 v);

extern void cpu_SetCP15Cnt(u32 v); //mask bit 1 for: 0 disable, 1 enable, PU
extern u32 cpu_GetCP15Cnt(); //get PU status: 0 disable, 1 enable

//instruction cache CP15
extern void IC_InvalidateAll();
extern void IC_InvalidateRange(const void *, u32 v);
extern void setitcmbase(); //@ ITCM base = 0 , size = 32 MB
extern void icacheenable(int);

//data cache CP15
extern void DC_FlushAll();
extern void DC_FlushRange(const void *, u32 v);
extern void setdtcmbase(); //@ DTCM base = __dtcm_start, size = 16 KB
extern void drainwrite();
extern void dcacheenable(int); //Cachability Bits for Data/Unified Protection Region (R/W)
extern void setgbamap();
extern u32 getdtcmbase();
extern u32 getitcmbase();
extern int setdtcmsz(u8 size);

#ifdef __cplusplus
}
#endif