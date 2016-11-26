//extra settings for buffer
#define sectorscale 1 //1,2,4,8
#define sectorsize 0x200*sectorscale
#define sectorbinsz clzero(sectorsize)

#define u32size (sizeof(u32))
#define u16size (sizeof(u16))
#define u8size (sizeof(u8))

#define sectorsize_u32units ((sectorsize/u32size)-1) //start from zero
#define sectorsize_u16units ((sectorsize/u16size)-1)
#define sectorsize_u8units ((sectorsize/u8size)-1)

#define sectorsize_int32units (sectorsize/u32size) 	//start from real number 1
#define sectorsize_int16units (sectorsize/u16size) 	//start from real number 1
#define sectorsize_int8units (sectorsize/u8size) 	//start from real number 1


#ifdef __cplusplus
extern "C" {
#endif

//disk buffer
extern volatile u32 disk_buf32[(sectorsize_int32units)]; 	//32 reads //0x80
extern volatile u16 disk_buf16[(sectorsize_int16units)];	//16 reads
extern volatile u8 disk_buf8[(sectorsize_int8units)];		//8 reads

//libfat / posix fat gba rom compatible
extern FILE * fsptr;

extern int startoffset32;	//start offset for disk_buf32 ,from gbabuffer fetch
extern int startoffset16;	//start offset for disk_buf32 ,from gbabuffer fetch
extern int startoffset8;	//start offset for disk_buf32 ,from gbabuffer fetch

u32 isgbaopen(FILE * gbahandler);
u32 opengbarom(const char * filename,const char * access_type); //char fn / POSIX r / rw / w
u32 closegbarom();
u32 readu32gbarom(int offset);
u16 readu16gbarom(int offset);
u8 readu8gbarom(int offset);

u16 writeu16gbarom(int offset,u16 * buf_in,int size_elem);
u32 writeu32gbarom(int offset,u32 * buf,int size);
u32 getfilesizegbarom();
u32 copygbablock(int offset,u8 mode); //mode 0:32b \ 1:16b \ 2:8b

#ifdef __cplusplus
}
#endif
