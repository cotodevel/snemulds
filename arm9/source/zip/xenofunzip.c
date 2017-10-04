/*
 * xenofunzip - gzip/zip decompression interface for zlib, based on info-zip's funzip
 * note: xenogzip uses gzio but xenofunzip uses normal inflateInit2()/inflate().
 */

#if defined(ARM9) || defined(ARM7)
#include "xenofunzip.h"

#include "typedefs.h"
#include "dsregs.h"
#include "console.h"

#include <stdlib.h>


#include "zlib.h"
#include "posix_hook_shared.h"	//add Toolchain Generic DS Filesystem Support
#include "mem_handler_shared.h"	//malloc support

void xfree(void *opaque, void *address){
	free(address);
}
void* xcalloc(void *opaque, unsigned items, unsigned size){
	uint8 * ret = malloc(items*size);
	return ret;
}

#else
//#include "../xenobox.h"
#define err(e,m) {fprintf_fs(stderr,"%s\n",m);return e;}
#endif

int funzipstdio(FILE *in, FILE *out){
	int encrypted;
	ush n;
	uch h[LOCHDR];                // first local header (GZPHDR < LOCHDR)
	int g = 0;                    // true if gzip format
	unsigned int method = 0;      // initialized here to shut up gcc warning
	int size = -1;

	//info-zip's funzip stuff
	n = fgetc_fs(in);  n |= fgetc_fs(in) << 8;
	if (n == ZIPMAG){
		if (fread_fs((char *)h, 1, LOCHDR, in) != LOCHDR || SH(h) != LOCREM)
			err(3, "invalid zipfile");
		switch (method = SH(h + LOCHOW)) {
			case STORED:
			case DEFLATED:
				break;
			default:
				err(3, "first entry not deflated or stored");
				break;
		}
		for (n = SH(h + LOCFIL); n--; ) g = fgetc_fs(in);
		for (n = SH(h + LOCEXT); n--; ) g = fgetc_fs(in);
		g = 0;
		size = LG(h+LOCSIZ);
		encrypted = h[LOCFLG] & CRPFLG;
	}else if (n == GZPMAG){
		if (fread_fs((char *)h, 1, GZPHDR, in) != GZPHDR)
			err(3, "invalid gzip file");
		if ((method = h[GZPHOW]) != DEFLATED && method != ENHDEFLATED)
			err(3, "gzip file not deflated");
		if (h[GZPFLG] & GZPMUL)
			err(3, "cannot handle multi-part gzip files");
		if (h[GZPFLG] & GZPISX){
			n = fgetc_fs(in);  n |= fgetc_fs(in) << 8;
			while (n--) g = fgetc_fs(in);
		}
		if (h[GZPFLG] & GZPISF)
			while ((g = fgetc_fs(in)) != 0 && g != EOF) ;
		if (h[GZPFLG] & GZPISC)
			while ((g = fgetc_fs(in)) != 0 && g != EOF) ;
		g = 1;
		encrypted = h[GZPFLG] & GZPISE;
	}else
		err(3, "input not a zip or gzip file");

	//now in points to deflated entry. let's just inflate it using zlib.

	//if entry encrypted, decrypt and validate encryption header
	if (encrypted)
		err(3, "encrypted zip unsupported");

	//decompress
	if (g || h[LOCHOW]){ //deflate
		Bytef *ibuffer, *obuffer;
		//uInt isize, osize;
		z_stream z;
		int result;

		z.zalloc = xcalloc;
		z.zfree = xfree;
		z.opaque = Z_NULL;
 
		result = inflateInit2( &z,-MAX_WBITS );
		if( result != Z_OK ) {
			err(result, z.msg );
		}

		ibuffer = xcalloc(NULL,1,BUFFER_SIZE);
		obuffer = xcalloc(NULL,1,BUFFER_SIZE);
    
		z.next_in = NULL;
		z.avail_in = 0;
		z.next_out = obuffer;
		z.avail_out = BUFFER_SIZE;
    
		for(;;){
			if( z.avail_in == 0 ){
				z.next_in = ibuffer;
				if(size>=0){
					if(size>0){
						z.avail_in = fread_fs( ibuffer, 1, min(size,BUFFER_SIZE), in );
						size-=min(size,BUFFER_SIZE);
					}
				}else{
					z.avail_in = fread_fs( ibuffer, 1, BUFFER_SIZE, in );
				}
			}

			result = inflate( &z, Z_SYNC_FLUSH ); //Z_NO_FLUSH? aww small buffer size...
			if( result != Z_OK && result != Z_STREAM_END ) {
				//free(ibuffer);free(obuffer);
				inflateEnd( &z );
				//char x[10];sprintf(x,"%d",result);
				//consoletext(64,x,0);while(1);
				err(result, z.msg );
			}
 
			fwrite_fs( obuffer, 1, BUFFER_SIZE - z.avail_out, out );
			z.next_out = obuffer;
			z.avail_out = BUFFER_SIZE;

			if(result==Z_STREAM_END)break;
		}
		//free(ibuffer);free(obuffer);
		inflateEnd( &z );
	}else{ //stored
		while (size--) {
			int c = fgetc_fs(in);fputc_fs(c,out);
		}
	}

	//should check CRC32 but...
	return 0;
}

/*
#if defined(ARM9) || defined(ARM7)
*/
int do_decompression(char *inname, char *outname){ //dszip frontend
	FILE *in=fopen_fs(inname,"r");
	if(!in)return -1;
	FILE *out=fopen_fs(outname,"w+");
	if(!out){fclose_fs(in);return -1;}
	int ret = funzipstdio(in,out);
	fclose_fs(in);fclose_fs(out);
	return ret;
}
/*
#else
int xenofunzip(const int argc, const char **argv){
	if(isatty(fileno(stdin))||isatty(fileno(stdout))){
		fprintf_fs(stderr,
			"xenofunzip - gzip/zip decompression interface for zlib\n"
			"based on info-zip's funzip\n"
			"Both stdin and stdout have to be redirected\n"
		);
		return -1;
	}
	return funzipstdio(stdin,stdout);
}
#endif
*/