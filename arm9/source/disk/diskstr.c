//coto: these offer a stream from memory cache mechanism rather than polling or some other method.
#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>//BRK(); SBRK();
#include <nds/ndstypes.h>
#include <nds/memory.h>
#include <fcntl.h>

#include "diskstr.h"

FILE * fsptr;

//[nds] disk sector ram buffer
volatile u32 disk_buf32[(sectorsize_int32units)]; 	//32 reads //0x80
volatile u16 disk_buf16[(sectorsize_int16units)];	//16 reads
volatile u8 disk_buf8[(sectorsize_int8units)];	//16 reads

int startoffset32=0;	//start offset for disk_buf32 ,from gbabuffer fetch
int startoffset16=0;	//start offset for disk_buf32 ,from gbabuffer fetch
int startoffset8=0;	//start offset for disk_buf32 ,from gbabuffer fetch

u32 isgbaopen(FILE * gbahandler){

	if (!gbahandler)
		return 1;
	else
		return 0;
}

/*
mode	Description
"r"	Open a file for reading. The file must exist.
"w"	Create an empty file for writing. If a file with the same name already exists its content is erased and the file is considered as a new empty file.
"a"	Append to a file. Writing operations append data at the end of the file. The file is created if it does not exist.
"r+"	Open a file for update both reading and writing. The file must exist.
"w+"	Create an empty file for both reading and writing.
"a+"	Open a file for reading and appending.
*/
u32 opengbarom(const char * filename,const char * access_type){

	FILE *fh = fopen(filename, access_type); //r
	if(!fh)
		return 1;

	fsptr=fh;
return 0;
}

u32 closegbarom(){
	if(!fsptr){
		iprintf("FATAL: GBAFH isn't open");
		return 1;
	}

	fclose(fsptr);

	iprintf("GBARom closed!");
return 0;
}

//offset to copy, mode=0 / 32bit | mode=1 / 16 bit buffer copies
u32 copygbablock(int offset,u8 mode){

	if(!fsptr){
		iprintf("FATAL: GBAFH isn't open");
		return 0;
	}
	
	int sizeread=0;
	
	switch(mode){
		//32bit copies
		case 0:{
			//1) from start of file where (offset)
			//int fseek(FILE *stream, long int offset, int whence);
			fseek(fsptr,(long int)offset, SEEK_SET); 					
		
			//2) perform read (512bytes read (128 reads))
			sizeread=fread((void*)disk_buf32, (int)u32size, (int)sectorsize_int32units ,fsptr);
		
			if (sizeread!=(int)sectorsize_int32units){
				iprintf("FATAL: GBAREAD isn't (%d) bytes",(int)sectorsize_int32units);
				return 0;
			}
			//3) and set pointer to what it was
			fseek(fsptr,0, SEEK_SET);									
		}
		break;
		
		//16bit copies
		case 1:{
			//1) from start of file where (offset)
			//int fseek(FILE *stream, long int offset, int whence);
			fseek(fsptr,(long int)offset, SEEK_SET); 					
		
			//2) perform read (512bytes read (256 reads))
			sizeread=fread((void*)disk_buf16, (int)u16size, (int)sectorsize_int16units ,fsptr);
		
			if (sizeread!=(int)sectorsize_int16units){
				iprintf("FATAL: GBAREAD isn't (%d) bytes, but: %x ",(int)sectorsize_int16units,(int)sizeread);
				return 0;
			}
			//3) and set pointer to what it was
			fseek(fsptr,0, SEEK_SET);									
		}
		break;
		
		//8bit copies
		case 2:{
			//1) from start of file where (offset)
			//int fseek(FILE *stream, long int offset, int whence);
			fseek(fsptr,(long int)offset, SEEK_SET); 					
		
			//2) perform read (512bytes read (256 reads))
			sizeread=fread((void*)disk_buf8, (int)u8size, (int)sectorsize_int8units ,fsptr);
		
			if (sizeread!=(int)sectorsize_int8units){
				iprintf("FATAL: GBAREAD isn't (%d) bytes, but: %x ",(int)sectorsize_int8units,(int)sizeread);
				return 0;
			}
			//3) and set pointer to what it was
			fseek(fsptr,0, SEEK_SET);
		}
		break;
		
		default:
			sizeread=0;
		break;
	}
	
return (u32)sizeread;
}

u8 readu8gbarom(int offset){
	if 	( 	(offset < (int)(startoffset8+sectorsize)) //starts from zero, so startoffset+sectorsize is next element actually((n-1)+1).
			&&
			(offset	>=	startoffset8)
		){
		return disk_buf8[ ((offset - startoffset8) / u8size) ]; //OK
	}
	else{
		int outblk=copygbablock(offset,2);
		if(((int)sectorsize_int8units)!=outblk){
			iprintf("\n readu8gbarom(); error copying romdata into disk_buf8");
			while(1);
			return 0;
		}
	
		startoffset8=offset;
		return disk_buf8[0]; //OK (must be zero since element 0 of the newly filled gbarom buffer has offset contents)
	}
return 0;
}

u16 readu16gbarom(int offset){

	if 	( 	(offset < (int)(startoffset16+sectorsize)) //starts from zero, so startoffset+sectorsize is next element actually((n-1)+1).
			&&
			(offset	>=	startoffset16)
		){
		
		return disk_buf16[ ((offset - startoffset16) / u16size) ]; //OK
	}
	else{
		int outblk=copygbablock(offset,1);
		if(((int)sectorsize_int16units)!=outblk){
			iprintf("\n readu16gbarom(); error copying romdata into disk_buf16");
			while(1);
			return 0;
		}
	
		startoffset16=offset;
		return disk_buf16[0]; //OK (must be zero since element 0 of the newly filled gbarom buffer has offset_start)
	}
return 0;
}


u32 readu32gbarom(int offset){

	if 	( 	(offset < (int)(startoffset32+sectorsize)) //starts from zero, so startoffset+sectorsize is next element actually((n-1)+1).
			&&
			(offset	>=	startoffset32)
		){
		return disk_buf32[ ((offset - startoffset32) / u32size) ]; //OK		
	}
	else{
		int outblk=copygbablock(offset,0);
		if(((int)sectorsize_int32units)!=outblk){
			iprintf("\n readu32gbarom(); error copying romdata into disk_buf32");
			while(1);
			return 0;
		}
	
		startoffset32=offset;
		return disk_buf32[0]; //OK (must be zero since element 0 of the newly filled gbarom buffer has offset_start)
	}
return 0;
}


u16 writeu16gbarom(int offset,u16 * buf_in,int size_elem){

	if(!fsptr){
		iprintf("FATAL: GBAFH isn't open");
		return 1;
	}
	iprintf("\n trying to write: %x",(unsigned int)buf_in[0x0]);

	//int fseek(FILE *stream, long int offset, int whence);
	fseek(fsptr,(long int)offset, SEEK_SET); 					//1) from start of file where (offset)

	//size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
	int sizewritten=fwrite((u16*)buf_in, 1, size_elem, fsptr); //2) perform read (512bytes read (128 reads))
	if (sizewritten!=size_elem){
		iprintf("FATAL: GBAWRITE isn't (%d) bytes, instead: (%x) bytes",(int)size_elem,(int)sizewritten);
	}
	else{
		iprintf("write ok!");
	}
	
	fseek(fsptr,0, SEEK_SET);									//3) and set pointer to what it was

return 0;
}


u32 writeu32gbarom(int offset,u32 * buf_in,int size_elem){

	if(!fsptr){
		iprintf("FATAL: GBAFH isn't open");
		return 1;
	}
	iprintf("\n trying to write: %x",(unsigned int)buf_in[0x0]);

	//int fseek(FILE *stream, long int offset, int whence);
	fseek(fsptr,(long int)offset, SEEK_SET); 					//1) from start of file where (offset)

	//size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
	int sizewritten=fwrite((u32*)buf_in, 1, size_elem, fsptr); //2) perform read (512bytes read (128 reads))
	if (sizewritten!=size_elem){
		iprintf("FATAL: GBAWRITE isn't (%d) bytes, instead: (%x) bytes",(int)size_elem,(int)sizewritten);
	}
	else{
		iprintf("write ok!");
	}
	
	fseek(fsptr,0, SEEK_SET);									//3) and set pointer to what it was

return 0;
}

u32 getfilesizegbarom(){
	if(!fsptr){
		iprintf("FATAL: GBAFH isn't open");
		return 0;
	}
	fseek(fsptr,0,SEEK_END);
	int filesize = ftell(fsptr);
	fseek(fsptr,0,SEEK_SET);

return filesize;
}