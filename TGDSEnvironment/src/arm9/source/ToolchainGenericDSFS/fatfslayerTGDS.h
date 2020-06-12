/*

			Copyright (C) 2017  Coto
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
USA

*/

//Coto: this was rewritten by me so it could fit the following setup:
//newlib libc nano ARM Toolchain. dirent.h is not supported in this newlib version so we restore it

#ifndef __fatfslayerTGDS_h__
#define __fatfslayerTGDS_h__

////////////////////////////////////////////////////////////////////////////INTERNAL CODE START/////////////////////////////////////////////////////////////////////////////////////

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include "devoptab_devices.h"
#include "posixHandleTGDS.h"
#include "ff.h"	//DIR struct definition here. dirent.h´s DIR was removed, and rewritten

//TGDS FS defs: These must be typecasted otherwise compiler will generate wrong strb opcodes
#define FT_NONE (int)(0)
#define FT_FILE (int)(1)
#define FT_DIR (int)(2)
#define FT_BUFFER (int)(3)

#define structfd_isused 	(sint32)(1)
#define structfd_isunused	(sint32)(0)
#define structfd_isattydefault	(sint32)(0)
#define structfd_descriptorflagsdefault	(sint32)(0)
#define structfd_status_flagsdefault	(sint32)(0)
#define structfd_fildir_offsetdefault	(sint32)(0)

//These must be invalid values so false positives do not arise in lookup functions
//FileList specific (internal retCode)

//these must be invalid values so false positives do not arise in lookup functions
#define structfd_posixInvalidFileDirOrBufferHandle	(sint32)(-1)	//used by fatfs_xxx layer, FAT_xxx layer (libfat wrapper), cluster functions. Marks a TGDS FileHandle as invalid (TGDS FileHandle: structFD index file descriptor)
#define structfd_posixExcludedFileDirOrBufferHandle	(sint32)(-2)	//used by fatfs_xxx layer, FAT_xxx layer (libfat wrapper), cluster functions. Marks a TGDS FileHandle as excluded and unused by TGDS FS API (TGDS FileHandle: structFD index file descriptor)

#define dirent_default_d_name	(sint8 *)("")
#define structfd_posixInvalidFileSize	(sint32)(-1)
#define structfd_posixInvalidFileHandleOffset	(sint32)(-1)	//internal offset currently held by the struct fd which exposes to POSIX API
#define structfd_FileClassListInvalidEntry		(sint32)(-1)	//TGDS FS API -> Directory Iterator (FileClass): Invalid Entry within a FileClassList

//libfat attributes so gccnewlibnano_to_fatfs is compatible with libfat homebrew
#ifndef ATTRIB_ARCH
#define ATTRIB_ARCH	(int)(0x20)			// Archive
#define ATTRIB_DIR	(int)(0x10)			// Directory
#define ATTRIB_LFN	(int)(0x0F)			// Long file name
#define ATTRIB_VOL	(int)(0x08)			// Volume
#define ATTRIB_SYS	(int)(0x04)			// System
#define ATTRIB_HID	(int)(0x02)			// Hidden
#define ATTRIB_RO	(int)(0x01)			// Read only
#endif

//up to N items to be listed by TGDS High level API functions
#define FileClassItems (int)(300)

//Directory Separator used by TGDS FS
#define TGDSDirectorySeparator ((char*)("/"))

//FileClass Start directory if there is none
#define FileClassStartDirectory TGDSDirectorySeparator

//FileClass parts (not used by POSIX at all, but ToolchainGenericDS high level API (for parsing fullpath directories and high level descriptors, and completely unrelated to TGDS FileOrBuffer handles)
struct FileClass{
	int type;	//FT_DIR / FT_FILE / FT_NONE	//  setup on Constructor / updated by getFileFILINFOfromPath(); / must be init from the outside 
	sint8 fd_namefullPath[MAX_TGDSFILENAME_LENGTH+1];
	bool isIterable;	//true = usable for buildfrompath / false = ignore lookup in buildfrompath
	int d_ino;	//if any, assign it here
	int curIndexInsideFileClassList;	//Current Index inside FileClassList
	struct FileClassList * parentFileClassList;	//Pointer to its parent FileClass List object
};

//FileClass List context. To handle TGDS FileSystem + Directory functions you need to define it.
struct FileClassList{
	//Current Working Directory held by the TGDS FS Directory Iterator (of which, is an instance)
	char TGDSCurrentWorkingDirectory[MAX_TGDSFILENAME_LENGTH+1];

	//The actual pointer inside the directory listing
	int CurrentFileDirEntry;	
	//These update on getFirstFile/Dir getNextFile/Dir
	int LastFileEntry;
	int LastDirEntry;
	int FileDirCount;	//actual Directory/File count
	struct FileClass fileList[FileClassItems];
};

struct fd {
	//If file handle: FILE/DIR Context in here
	FIL  * filPtr;	//reuse C ability to parse NULL structs later
	DIR  * dirPtr;
	FIL  fil;	//if aboveptr is NULL then it is not FIL
	DIR  dir;	//if aboveptr is NULL then it is not DIR
	bool fileIsUnicode;	//true: file is Unicode, false file is not (just standard ASCII encoding)
	struct devoptab_t * devoptabFileDescriptor;
	int isatty;
	int isused;					//1 means in use (structfd_isused) / 0 free (structfd_isunused)
	int descriptor_flags;
	int status_flags;
	struct dirent cur_entry;	//dirent POSIX extension used in fopen / fatfs_readdir_r
	sint8 fd_name[MAX_TGDSFILENAME_LENGTH+1];	/* d_name dirent but holds full file path. Only when this is a file */
	
	//If buffer mapped as file handle:
	u8* bufferFD;
	int bufferFDSize;
	
	//Shared
	struct stat stat;			//File attributes : POSIX Compliant
	long loc;		//current file pointer if this struct fd is DIR * (generated by opendir) Note: Only valid if struct fd == FT_FILE. Otherwise it has no effect. 
	int StructFDType; //FT_DIR / FT_FILE / FT_NONE
	int StructFD;	//TGDS File Descriptor (struct fd). Identical as d_ino (POSIX File Descriptor), except, it is valid if the file context is correct from FATFS API.
	//filehandle -> fileSize;	//use f_size(filPtr)
};


#ifdef __cplusplus
extern "C" {
#endif

// Function prototypes 
extern FATFS dldiFs;
extern bool FS_InitStatus;

////////////////////////////////////////////////////////////////////////////USER CODE START/////////////////////////////////////////////////////////////////////////////////////
extern int		FS_init();
extern int		FS_deinit();
extern sint8 charbuf[MAX_TGDSFILENAME_LENGTH+1];
extern sint8 * getfatfsPath(sint8 * filename);
extern int FileExists(char * filename);
extern int rename(const sint8 *oldpathfile, const sint8 *newpathfile);
extern int fsync(int fd);
extern int mkdir(const sint8 *path, mode_t mode);
extern int rmdir(const sint8 *path);
extern int chdir(const sint8 *path);
extern sint8 *getcwd(sint8 *buf, size_t size);
extern DIR *opendir(const sint8 *path);
extern int closedir(DIR *dirp);
extern struct dirent *readdir(DIR *dirp);
extern void rewinddir(DIR *dirp);
extern int dirfd(DIR *dirp);
extern int remove(const char *filename);
extern int chmod(const char *pathname, mode_t mode);
extern DIR *fdopendir(int fd);
extern void seekdir(DIR *dirp, long loc);
extern int fatfs2libfatAttrib(int fatfsFlags);
extern int libfat2fatfsAttrib(int libfatFlags);
extern void SetfatfsAttributesToFile(char * filename, int Newgccnewlibnano_to_fatfsAttributes, int mask);

extern DWORD clust2sect (  /* !=0:Sector number, 0:Failed (invalid cluster#) */
    FATFS* fs,      /* File system object */
    DWORD clst      /* Cluster# to be converted */
);

extern uint32 getStructFDFirstCluster(struct fd *fdinst);
extern uint32 getStructFDNextCluster(struct fd *fdinst, int currCluster);
extern uint32 getStructFDStartSectorByCluster(struct fd *fdinst, int ClusterIndex);
extern sint32 getDiskClusterSize();			/* Cluster size [sectors] */
extern sint32 getDiskClusterSizeBytes();	/* Cluster size [sectors] in bytes */
extern sint32 getDiskSectorSize();
extern char * dldi_tryingInterface();

/////////////////////////////////////////Libfat wrapper layer. Call these as if you were calling libfat code./////////////////////////////////////////
extern struct FileClass * FAT_FindFirstFile(char* filename, struct FileClassList * lst, int startFromGivenIndex);
extern struct FileClass * 	FAT_FindNextFile(char* filename, struct FileClassList * lst);
extern u8 	FAT_GetFileAttributes(struct FileClassList * lst);
extern u8 	FAT_SetFileAttributes(const char* filename, u8 attributes, u8 mask);
extern bool readFileNameFromFileClassIndex(char* filename_out, struct FileClass * FileClassInst);
extern bool FAT_GetAlias(char* alias, struct FileClassList * lst);
extern void FAT_preserveVars();
extern void FAT_restoreVars();
extern u32	disc_HostType(void);
extern bool FAT_GetLongFilename(char* Longfilename, struct FileClassList * lst);
extern u32 	FAT_GetFileSize(struct FileClassList * lst);
extern u32 	FAT_GetFileCluster(struct FileClassList * lst);
extern bool disableWriting;
extern void FAT_DisableWriting (void);
extern int 	FAT_FileExists(char* filename);
extern bool FAT_FreeFiles (void);
extern bool FAT_InitFiles (void);
/////////////////////////////////////////////Libfat wrapper layer End/////////////////////////////////////////////

extern struct FileClass * getFirstFile(char * path, struct FileClassList * lst, int startFromGivenIndex);
extern struct FileClass * getNextFile(char * path, struct FileClassList * lst);
extern int	getCurrentDirectoryCount(struct FileClassList * lst);
extern void	setCurrentDirectoryCount(struct FileClassList * lst, int value);
extern bool	getFileFILINFOfromFileClass(struct FileClass * fileInst, FILINFO * finfo);
extern struct FileClass getFileClassFromPath(char * path);
extern struct FileClass * getFirstDirEntryFromList(struct FileClassList * lst);
extern struct FileClass * getFirstFileEntryFromList(struct FileClassList * lst);
extern struct FileClass * getFileClassFromList(int FileClassListIndex, struct FileClassList * lst);
extern bool setFileClass(bool iterable, char * fullPath, int FileClassListIndex, int Typ, int StructFD, struct FileClassList * lst);
extern bool TGDSFS_detectUnicode(struct fd *pfd);
////////////////////////////////////////////////////////////////////////////USER CODE END/////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////INTERNAL CODE START/////////////////////////////////////////////////////////////////////////////////////
extern int fatfs_init();
extern int fatfs_deinit();
extern int fatfs_write (int fd, sint8 *ptr, int len);
extern int fatfs_read (int fd, sint8 *ptr, int len);
extern int fatfs_close (int fd);
extern int fatfs_fsize (int structFDIndex);
extern void fillPosixStatStruct(const FILINFO *fno, struct stat *out);
extern BYTE posixToFatfsAttrib(int flags);
extern int fatfsToPosixAttrib(BYTE flags);
extern int fresultToErrno(FRESULT result);	/* POSIX ERROR Handling */
extern void initStructFDHandle(struct fd *pfd, int flags, const FILINFO *fno, int structFD, int StructFDType);

extern int fatfs_open_file(const sint8 *pathname, int flags, const FILINFO *fno);	//(FRESULT is the file properties that must be copied to stat st)/ returns an internal index struct fd allocated
extern int fatfs_open_dir(const sint8 *pathname, int flags, const FILINFO *fno);	//(FRESULT is the file properties that must be copied to stat st)/ returns an internal index struct fd allocated
extern int fatfs_open_file_or_dir(const sint8 *pathname, int flags);	//returns an internal index struct fd allocated
extern DWORD get_fattime (void);
extern int fatfs_open(const sint8 *pathname, int flags);
extern off_t fatfs_lseek(int fd, off_t offset, int whence );
extern int fatfs_unlink(const sint8 *path);
extern int fatfs_link(const sint8 *path1, const sint8 *path2);
extern int fatfs_rename(const sint8 *oldpathfile, const sint8 *newpathfile);
extern int fatfs_fsync(int fd);
extern int fatfs_stat(const sint8 *path, struct stat *buf);
extern int fatfs_mkdir(const sint8 *path, mode_t mode);
extern int fatfs_rmdir(const sint8 *path);
extern int fatfs_chdir(const sint8 *path);
extern sint8 *fatfs_getcwd(sint8 *buf, size_t size);
extern DIR *fatfs_opendir(const sint8 *path);
extern int fatfs_closedir(DIR *dirp);
extern int fatfs_dirfd(DIR *dirp);
extern DIR *fatfs_fdopendir(int fd);
extern struct dirent *fatfs_readdir(DIR *dirp);
extern int fatfs_readdir_r(DIR *dirp,struct dirent *entry,struct dirent **result);
extern void fatfs_rewinddir(DIR *dirp);
extern long fatfs_tell(struct fd *f);
extern void fatfs_seekdir(DIR *dirp, long loc);
extern int _fstat_r ( struct _reent *_r, int fd, struct stat *buf );
extern int  readdir_r(DIR * dirp,struct dirent * entry,struct dirent ** result);
extern int OpenFileFromPathGetStructFD(char * fullPath);
extern bool closeFileFromStructFD(int StructFD);
extern struct fd * files;	//File/Dir MAX handles: OPEN_MAXTGDS

////////////////////////////////////////////////////////////////////////////INTERNAL CODE END/////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS /////////////////////////////////////////////////////////////////////
extern bool buildFileClassListFromPath(char * path, struct FileClassList * lst, int startFromGivenIndex);
extern bool enterDir(char* newDir, char* destinationPath);
extern bool leaveDir(char* oldNewDir);
extern bool updateTGDSFSDirectoryIteratorCWD(char * newCWD, struct FileClassList * lst);
/////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS END //////////////////////////////////////////////////////////////////

//Link C to C++ related dependencies
extern FILE *fdopen(int fd, const char *mode);
extern int fileno(FILE *);
extern bool setFileClassObj(int FileClassListIndex, struct FileClass * FileClassObj, struct FileClassList * lst);

#ifdef __cplusplus
}
#endif


//Takes an open file handle, gets filesize without updating its internal file pointer
static inline int	FS_getFileSizeFromOpenHandle(FILE * f){
	int size = -1;
	if (f != NULL){
		int fLoc = ftell(f);
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, fLoc, SEEK_SET);
	}
	return size;
}

static inline int	FS_getFileSizeFromOpenStructFD(struct fd * tgdsfd){
	int size = -1;
	if ((tgdsfd != NULL) && (tgdsfd->filPtr != NULL)){
		size = f_size(tgdsfd->filPtr);
	}
	return size;
}

static inline __attribute__((always_inline))
struct fd *getStructFD(int fd){
    struct fd *f = NULL;
    if((fd < OPEN_MAXTGDS) && (fd >= 0)){
        f = (struct fd *)(files + fd);
		if(f->dirPtr){
			f->dirPtr = (DIR *)&(files + fd)->dir;
		}
		if(f->filPtr){
			f->filPtr = (FIL *)&(files + fd)->fil;
		}
    }
    return f;
}

//char * devoptabFSName must be a buffer already allocated if bool defaultDriver == false
static inline __attribute__((always_inline))
void initTGDS(char * devoptabFSName){
	if(devoptabFSName == NULL){
		return;
	}
	
	initTGDSDevoptab();
	
	int fd = 0;
	/* search in all struct fd instances*/
	for (fd = 0; fd < OPEN_MAXTGDS; fd++){
		memset((uint8*)(files + fd), 0, sizeof(struct fd));
		(files + fd)->isused = (sint32)structfd_isunused;
		//Internal default invalid value (overriden later)
		(files + fd)->cur_entry.d_ino = (sint32)structfd_posixInvalidFileDirOrBufferHandle;	//Posix File Descriptor
		(files + fd)->StructFD = (sint32)structfd_posixInvalidFileDirOrBufferHandle;		//TGDS File Descriptor (struct fd)
		(files + fd)->StructFDType = FT_NONE;	//struct fd type invalid.
		(files + fd)->isatty = (sint32)structfd_isattydefault;
		(files + fd)->descriptor_flags = (sint32)structfd_descriptorflagsdefault;
		(files + fd)->status_flags = (sint32)structfd_status_flagsdefault;
		(files + fd)->loc = (sint32)structfd_fildir_offsetdefault;
		
		(files + fd)->filPtr = NULL;
		(files + fd)->dirPtr = NULL;
	}
	//Set up proper devoptab device mount name.
	memcpy((uint32*)&devoptab_sdFilesystem.name[0], (uint32*)devoptabFSName, strlen(devoptabFSName));
}

static inline __attribute__((always_inline))
FILE * getPosixFileHandleByStructFD(struct fd * fdinst, const char * mode){
	if(fdinst != NULL){
		return fdopen(fdinst->cur_entry.d_ino, mode);
	}
	return NULL;
}

static inline __attribute__((always_inline))
struct fd * getStructFDByFileHandle(FILE * fh){
	if(fh != NULL){
		int StructFD = fileno(fh);
		return getStructFD(StructFD);
	}
	return NULL;
}

static inline __attribute__((always_inline))
int getStructFDIndexByFileName(char * filename){
	int ret = structfd_posixInvalidFileDirOrBufferHandle;
	int fd = 0;
	/* search in all struct fd instances*/
	for (fd = 0; fd < OPEN_MAXTGDS; fd++){
		if((files + fd)->isused == (sint32)structfd_isused){
			if(strcmp((char*)&(files + fd)->fd_name, filename) == 0){
				//printfDebugger("getStructFDIndexByFileName(): idx:%d - f:%s",fd, (files + fd)->fd_name);
				return fd;
			}
		}
	}
	return ret;
}

//returns / allocates a new struct fd index ,  for a certain devoptab_t so we can allocate different handles for each devoptab
static inline __attribute__((always_inline))
int FileHandleAlloc(struct devoptab_t * devoptabInst){
    int fd = 0;
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    for (fd = 0; fd < OPEN_MAXTGDS; fd++){
        if ((sint32)(files + fd)->isused == (sint32)structfd_isunused){
			(files + fd)->isused = (sint32)structfd_isused;
			
			//PosixFD default valid value (overriden now)
			(files + fd)->devoptabFileDescriptor = devoptabInst;
			
			//Internal default value (overriden now)
			(files + fd)->cur_entry.d_ino = (sint32)fd;	//Posix File Descriptor
			(files + fd)->StructFD = (sint32)structfd_posixInvalidFileDirOrBufferHandle;		//TGDS File Descriptor (struct fd) not valid yet. Set up by initStructFDHandle()
			(files + fd)->StructFDType = FT_NONE;	//struct fd type not valid yet. Will be assigned by fatfs_open_file (FT_FILE), or fatfs_open_dir (FT_DIR)
			
			(files + fd)->isatty = (sint32)structfd_isattydefault;
			(files + fd)->descriptor_flags = (sint32)structfd_descriptorflagsdefault;
			(files + fd)->status_flags = (sint32)structfd_status_flagsdefault;
			(files + fd)->loc = (sint32)structfd_fildir_offsetdefault;
			
			(files + fd)->filPtr = NULL;
			(files + fd)->dirPtr = NULL;
			
            ret = fd;
            break;
        }
    }
	//if for some reason all the file handles are exhausted, discard the last one and assign that one. (fixes homebrew that opens a lot of file handles and doesn't close them up accordingly)
	if(ret == structfd_posixInvalidFileDirOrBufferHandle){
		int retClose = fatfs_close(OPEN_MAXTGDS - 1);	//requires a struct fd(file descriptor), returns 0 if success, structfd_posixInvalidFileDirOrBufferHandle if error
		if(retClose == structfd_posixInvalidFileDirOrBufferHandle){
			//couldnt really close file handle
			ret = retClose;
		}
		else{
			//file handle close success!
			return FileHandleAlloc(devoptabInst);	//ret == return value here
		}
	}

    return ret;
}

//deallocates a posix index, returns such index deallocated
static inline __attribute__((always_inline))
int FileHandleFree(int fd){
	int ret = structfd_posixInvalidFileDirOrBufferHandle;
    if ((fd < OPEN_MAXTGDS) && (fd >= 0) && ((files + fd)->isused == structfd_isused)){
        (files + fd)->isused = (sint32)structfd_isunused;
		(files + fd)->cur_entry.d_ino = (sint32)structfd_posixInvalidFileDirOrBufferHandle;	//Posix File Descriptor
		(files + fd)->StructFD = (sint32)structfd_posixInvalidFileDirOrBufferHandle;		//TGDS File Descriptor (struct fd)
		(files + fd)->StructFDType = FT_NONE;	//struct fd type invalid.
		(files + fd)->loc = structfd_posixInvalidFileHandleOffset;
		ret = fd;
    }
	return ret;
}

static inline __attribute__((always_inline))
sint8 * getDeviceNameByStructFDIndex(int StructFDIndex){
	sint8 * out;
	if((StructFDIndex < 0) || (StructFDIndex > OPEN_MAXTGDS)){
		out = NULL;
	}
	
	sint32 i = 0;
	/* search in all struct fd instances*/
    for (i = 0; i < OPEN_MAXTGDS; i++)
    {
        if ((files + i)->cur_entry.d_ino == StructFDIndex)
        {
			out = (sint8*)(&devoptab_struct[i]->name);
		}
    }
	return out;
}

//useful for handling native DIR * to Internal File Descriptors (struct fd index)
static inline __attribute__((always_inline))
int getStructFDIndexByDIR(DIR *dirp){
	int fd = 0;
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    /* search in all struct fd instances*/
    for (fd = 0; fd < OPEN_MAXTGDS; fd++)
    {
        if ((files + fd)->dirPtr)
        {
			if((files + fd)->dirPtr->obj.id == dirp->obj.id){
				ret = (files + fd)->cur_entry.d_ino;
				break;
			}
		}
    }
	return ret;
}

static inline struct FileClassList * initFileList(){
	return (struct FileClassList *)TGDSARM9Malloc(sizeof(struct FileClassList));
}

//return: true if clean success
//false if lst == NULL
static inline bool cleanFileList(struct FileClassList * lst){
	if(lst != NULL){
		memset((u8*)lst, 0, sizeof(struct FileClassList));
		lst->CurrentFileDirEntry = 0;
		lst->LastDirEntry=structfd_posixInvalidFileDirOrBufferHandle;
		lst->LastFileEntry=structfd_posixInvalidFileDirOrBufferHandle;
		setCurrentDirectoryCount(lst, 0);
		return true;
	}
	return false;
}

static inline void freeFileList(struct FileClassList * lst){
	if(lst != NULL){
		TGDSARM9Free(lst);
	}
}

//returns: the struct FileClassList * context if success
//if error: returns NULL, which means, operation failed.

static inline struct FileClassList * pushEntryToFileClassList(bool iterable, char * fullPath, int Typ, int StructFD, struct FileClassList * lst){
	if(lst != NULL){		
		int FileClassListIndex = lst->FileDirCount;
		setFileClass(iterable, fullPath, FileClassListIndex, Typ, StructFD, lst);
		return lst;
	}
	return NULL;
}

static inline struct FileClassList * popEntryfromFileClassList(struct FileClassList * lst){
	if(lst != NULL){		
		int FileClassListIndex = lst->FileDirCount;
		if(FileClassListIndex > 0){
			if(FileClassListIndex < FileClassItems){	//prevent overlapping current FileClassList 
				lst->FileDirCount = FileClassListIndex - 1;
				struct FileClass * FileClassInst = getFileClassFromList(FileClassListIndex, lst);
				memset(FileClassInst, 0, sizeof(struct FileClass));
				return lst;
			}
		}
	}
	return NULL;
}

static inline bool contains(int * arr, int arrSize, int value){
    int i=0;
	for(i=0; i < arrSize; i++){
        if(arr[i] == value){
            return true;
        }
    }
    return false;
} 

static inline struct FileClassList * randomizeFileClassList(struct FileClassList * lst){
	if(lst != NULL){
		//Build rand table
		int listCount = lst->FileDirCount;
		int uniqueArr[FileClassItems];
		int i=0;
		for(i=0; i < FileClassItems; i++){
            uniqueArr[i] = -1;
        }
		int indx=0;
		for(;;){
		    int randVal = rand() % (listCount);
		    if (contains(uniqueArr, listCount, randVal) == false){
		        uniqueArr[indx] = randVal;
		        indx++;
		    }
		    if(indx == (listCount)){
		        break;
		    }
		}
		
		//Shuffle it
		for(indx=0; indx < listCount; indx++){
			struct FileClass fileListSrc;
			struct FileClass fileListTarget;
			memset(&fileListSrc, 0, sizeof(struct FileClass));
			memset(&fileListTarget, 0, sizeof(struct FileClass));
			
			int targetIndx = uniqueArr[indx];
			int srcIndx = indx;
			
			if(targetIndx != srcIndx){
				struct FileClass * FileClassInstSource = getFileClassFromList(srcIndx, lst);
				struct FileClass * FileClassInstTarget = getFileClassFromList(targetIndx, lst);
				
				memcpy((u8*)&fileListSrc, (u8*)FileClassInstSource, sizeof(struct FileClass));
				memcpy((u8*)&fileListTarget, (u8*)FileClassInstTarget, sizeof(struct FileClass));
				
				memcpy((u8*)FileClassInstSource, (u8*)&fileListTarget, sizeof(struct FileClass));
				memcpy((u8*)FileClassInstTarget, (u8*)&fileListSrc, sizeof(struct FileClass));
			}
		}
		
		//done 
		return lst;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////INTERNAL CODE END/////////////////////////////////////////////////////////////////////////////////////

#endif
