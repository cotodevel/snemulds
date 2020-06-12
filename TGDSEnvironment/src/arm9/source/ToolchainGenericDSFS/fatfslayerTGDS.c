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
//newlib libc ARM Toolchain. <dirent.h> implementation is platform-specific, thus, implemented for ToolchainGenericDS.

#include "fatfslayerTGDS.h"
#include "dldi.h"
#include "fileBrowse.h"	//generic template functions from TGDS: maintain 1 source, whose changes are globally accepted by all TGDS Projects.

//fatfs
FATFS dldiFs;

/* functions */
bool FS_InitStatus = false;	

// Physical struct fd file handles
struct fd * files = NULL;	//File/Dir MAX handles: OPEN_MAXTGDS

////////////////////////////////////////////////////////////////////////////USER CODE START/////////////////////////////////////////////////////////////////////////////////////

//Usercall: For initializing Filesystem:
//if FS_init() init SD equals true: Init success
//else  FS_init() equals false: Could not init the card SD access 
int		FS_init(){
	int ret = fatfs_init();
	if (ret == 0){
		FS_InitStatus = true;
	}
	else{
		FS_InitStatus = false;
	}
	return ret;
}

//Usercall: For de-initializing Filesystem
//if FS_deinit() or sd driver uninitialized (after calling FS_init()) equals true: Deinit sucess
//else  FS_deinit() equals false: Could not deinit/free the card SD access 
int		FS_deinit(){
	int ret = fatfs_deinit();
	if (ret == 0){
		FS_InitStatus = false;
	}
	return ret;
}

//converts a "folder/folder.../file.fil" into a proper filesystem fullpath
sint8 charbuf[MAX_TGDSFILENAME_LENGTH+1];
sint8 * getfatfsPath(sint8 * filename){
	sprintf((sint8*)charbuf,"%s%s",devoptab_sdFilesystem.name,filename);
	return (sint8*)&charbuf[0];
}


//these two work together (usually)
int OpenFileFromPathGetStructFD(char * fullPath){
	FILE* fil = fopen(fullPath,"r");
	if(fil != NULL){
		return fileno(fil);
	}
	return structfd_posixInvalidFileDirOrBufferHandle;
}
bool closeFileFromStructFD(int StructFD){
	FILE* fh = fdopen(StructFD, "r");
	if(fh != NULL){
		fclose(fh);
		return true;
	}
	return false;
}

//retcode: FT_NONE , FT_DIR or FT_FILE
int FileExists(char * filename){
	int Type = FT_NONE;
	int StructFD = OpenFileFromPathGetStructFD((char *)filename);
	struct fd *pfd = getStructFD(StructFD);
	if(pfd != NULL){
		//file?
		if(S_ISREG(pfd->stat.st_mode)){
			Type = FT_FILE;
		}
		//dir?
		else if(S_ISDIR(pfd->stat.st_mode)){
			Type = FT_DIR;
		}
		closeFileFromStructFD(StructFD);
	}
	return Type;
}

int rename(const sint8 *oldfname, const sint8 *newfname){
    return fatfs_rename(oldfname, newfname);
}

int fsync(int structFDIndex){	//(FileDescriptor :struct fd index)
    return fatfs_fsync(structFDIndex);
}

int mkdir(const sint8 *path, mode_t mode){
    return fatfs_mkdir(path, mode);
}

int rmdir(const sint8 *path){
    return fatfs_rmdir(path);
}

int chdir(const sint8 *path){
    return fatfs_chdir(path);
}

sint8 *getcwd(sint8 *buf, size_t size){
    return fatfs_getcwd(buf, size);
}

DIR *opendir(const sint8 *path){
	return fatfs_opendir(path);
}

int closedir(DIR *dirp){
    return fatfs_closedir(dirp);
}

struct dirent *readdir(DIR *dirp){
    return fatfs_readdir(dirp);
}

void rewinddir(DIR *dirp){
    fatfs_rewinddir(dirp);
}

int dirfd(DIR *dirp){
    return fatfs_dirfd(dirp);
}

int remove(const char *filename){
	return fatfs_unlink((const sint8 *)filename);
}


int chmod(const char *pathname, mode_t mode){
	BYTE fatfsFlags = posixToFatfsAttrib(mode);
	return f_chmod(pathname, fatfsFlags, AM_SYS );	//only care about the system bit (if the file we are changing is SYSTEM)
}

DIR *fdopendir(int structFDIndex){	//(FileDescriptor :struct fd index)
    return fatfs_fdopendir(structFDIndex);
}

void seekdir(DIR *dirp, long loc){
    fatfs_seekdir(dirp, loc);
}

//Input: libfat directory flags
//Output: FILINFO.fattrib 
int libfat2fatfsAttrib(int libfatFlags){
	int fatfsFlags = 0;
	if(libfatFlags & ATTRIB_RO){	/* Read only */
		fatfsFlags|=AM_RDO;
	}
	if(libfatFlags & ATTRIB_HID){	/* Hidden */
		fatfsFlags|=AM_HID;
	}	
	if(libfatFlags & ATTRIB_SYS){	/* System */
		fatfsFlags|=AM_SYS;
	}
	if(libfatFlags & ATTRIB_DIR){	/* Directory */
		fatfsFlags|=AM_DIR;
	}
	if(libfatFlags & ATTRIB_ARCH){	/* Archive */
		fatfsFlags|=AM_ARC;
	}
	return fatfsFlags;
}

//Input: FILINFO.fattrib 
//Output: libfat directory flags
int fatfs2libfatAttrib(int fatfsFlags){
	int libfatFlags = 0;
	if(fatfsFlags & AM_RDO){	/* Read only */
		libfatFlags|=ATTRIB_RO;
	}
	if(fatfsFlags & AM_HID){	/* Hidden */
		libfatFlags|=ATTRIB_HID;
	}
	if(fatfsFlags & AM_SYS){	/* System */
		libfatFlags|=ATTRIB_SYS;
	}
	if(fatfsFlags & AM_DIR){	/* Directory */
		libfatFlags|=ATTRIB_DIR;
	}
	if(fatfsFlags & AM_ARC){	/* Archive */
		libfatFlags|=ATTRIB_ARCH;
	}
	return libfatFlags;
}

void SetfatfsAttributesToFile(char * filename, int Newgccnewlibnano_to_fatfsAttributes, int mask){
	f_chmod(filename, Newgccnewlibnano_to_fatfsAttributes, mask);
}

//posix -> fatfs
BYTE posixToFatfsAttrib(int flags){
    BYTE mode = 0;
    int accmode = flags & O_ACCMODE;
    if ((accmode == O_RDONLY) || (accmode == O_RDWR)){
        mode |= FA_READ;
    }
    if ((accmode == O_WRONLY) || (accmode == O_RDWR)){
        mode |= FA_WRITE;

        if (!(flags & O_CREAT))
        {
            mode |= FA_OPEN_EXISTING;
        }
        else if (flags & O_EXCL)
        {
            mode |= FA_CREATE_NEW;
        }
        else if ((flags & O_TRUNC) || (flags & O_CREAT))
        {
            mode |= FA_CREATE_ALWAYS;
        }
        else
        {
            mode |= FA_OPEN_ALWAYS;
        }
    }
    return mode;
}

//fatfs -> posix
int fatfsToPosixAttrib(BYTE flags){
    #define fatfs_O_ACCMODE (FA_READ|FA_WRITE)
	int mode = 0;
    BYTE accmode = flags & fatfs_O_ACCMODE;
    if (accmode & FA_READ){
        mode |= O_RDONLY;
    }
	if(accmode & FA_OPEN_EXISTING){
		mode |= O_RDWR;
	} 
    if (accmode == FA_CREATE_NEW){
        mode |= O_WRONLY;
        if (!(flags & FA_OPEN_EXISTING)){
            mode |= O_CREAT;
        }
        else if (flags & FA_CREATE_NEW){
            mode |= O_EXCL;
        }
        else if (flags & FA_CREATE_ALWAYS){
            mode |= O_TRUNC;
        }
        else{
            mode |= O_RDONLY;
        }
    }
    return mode;
}

/* POSIX ERROR Handling */
int fresultToErrno(FRESULT result){
    int err ;
    switch (result)
    {
        case FR_OK:
            err = 0;
            break;
        case FR_EXIST:
            err = EEXIST;
            break;
        case FR_NO_FILE:
            err = ENOENT;
            break;
        case FR_NO_PATH:
        case FR_INVALID_NAME:
            err = ENOTDIR;
            break;
        case FR_WRITE_PROTECTED:
            err = EROFS;
            break;
        case FR_DENIED:
            err = EACCES;
            break;
        case FR_TOO_MANY_OPEN_FILES:
            err = ENFILE;
            break;
        case FR_NOT_ENOUGH_CORE:
            err = ENOMEM;
            break;
        case FR_DISK_ERR:
        case FR_INT_ERR:
        case FR_NOT_READY:
        case FR_INVALID_OBJECT:
        case FR_INVALID_DRIVE:
        case FR_NOT_ENABLED:
        case FR_NO_FILESYSTEM:
        case FR_MKFS_ABORTED:
        case FR_TIMEOUT:
        case FR_LOCKED:
        case FR_INVALID_PARAMETER:
        /* TODO */
        default:
            err = EINVAL;
            break;
    }

    return err;
}

DWORD clust2sect (  /* !=0:Sector number, 0:Failed (invalid cluster#) */
    FATFS* fs,      /* File system object */
    DWORD clst      /* Cluster# to be converted */
)
{
    clst -= 2;
    if (clst >= fs->n_fatent - 2) return 0;       /* Invalid cluster# */
    return clst * fs->csize + fs->database;
}

//FATFS: The file handle start cluster is in fp->obj.sclust as long as you haven't read from that file (otherwise its in fp->clust)
uint32	getStructFDFirstCluster(struct fd *fdinst){
	if(fdinst->filPtr){
		if( (int)fdinst->filPtr->fptr == (int)0 ){
			return (int)(fdinst->filPtr->obj.sclust);
		}
		else{
			return (int)(fdinst->filPtr->clust);
		}
	}
	return (uint32)structfd_posixInvalidFileDirOrBufferHandle;
}

uint32	getStructFDNextCluster(struct fd *fdinst, int currCluster){
	if((fdinst != NULL) && (fdinst->filPtr)){
		DWORD clst = getStructFDFirstCluster(fdinst);
		FIL * fil = fdinst->filPtr;
		FATFS *fs = fil->obj.fs;
		clst = get_fat(&fil->obj, (DWORD)((int)clst + (int)currCluster) );		/* Get next cluster */
		if ( (clst == 0xFFFFFFFF) || (clst < 2) || (clst >= fs->n_fatent) ){ 	/* Disk error , or  Reached to end of table or internal error */
			return (uint32)structfd_posixInvalidFileDirOrBufferHandle;	
		}
		else{
			return (sint32)clst;
		}
	}
	return (uint32)structfd_posixInvalidFileDirOrBufferHandle;
}

//Returns the First sector pointed at ClusterIndex, or, structfd_posixInvalidFileDirOrBufferHandle if fails.
uint32 getStructFDStartSectorByCluster(struct fd *fdinst, int ClusterIndex){	//	struct File Descriptor (FILE * open through fopen() -> then converted to int32 from fileno())
	if(fdinst->filPtr != NULL){
		return (uint32)(clust2sect(fdinst->filPtr->obj.fs, getStructFDFirstCluster(fdinst) + ClusterIndex));
	}
	return structfd_posixInvalidFileDirOrBufferHandle;
}

sint32 getDiskClusterSize(){		/* Sectors per Cluster */
	return(sint32)(dldiFs.csize);
}

sint32 getDiskClusterSizeBytes(){	/* Sectors per Cluster in bytes */
	return (sint32)(getDiskClusterSize() * getDiskSectorSize());
}

sint32 getDiskSectorSize(){
	int diskSectorSize = 0;
	if(FF_MAX_SS == FF_MIN_SS){
		diskSectorSize = (int)FF_MIN_SS;
	}
	else{
		#if (FF_MAX_SS != FF_MIN_SS)
		diskSectorSize = dldiFs.ssize;	//when fatfs sectorsize is variable, by default its 512
		#endif
	}
	return diskSectorSize;
}

char * dldi_tryingInterface(){
	//DS DLDI
	struct  DLDI_INTERFACE* dldiInterface = (struct  DLDI_INTERFACE*)DLDIARM7Address;
	return (char *)&dldiInterface->friendlyName[0];
}


///////////////////////////////////This is the TGDS FS API extension. It emulates libfat FAT_xxx functions.//////////////////////////////////
/////////////// For an example, please refer to https://bitbucket.org/Coto88/toolchaingenericds-ndstools/src , main.cpp file/////////////////


/////////////////////////////////////////Libfat wrapper layer. Call these as if you were calling libfat code./////////////////////////////////////////

//Filename must be at least MAX_TGDSFILENAME_LENGTH+1 in size
struct FileClass * FAT_FindFirstFile(char* filename, struct FileClassList * lst, int startFromGivenIndex){	
	return getFirstFile(filename, lst, startFromGivenIndex);
}

//Filename must be at least MAX_TGDSFILENAME_LENGTH+1 in size
struct FileClass * FAT_FindNextFile(char* filename, struct FileClassList * lst){
	return getNextFile(filename, lst);
}

u8 FAT_GetFileAttributes(struct FileClassList * lst){
	u8	libfatAttributes = 0;
	int curFileDirEntry = lst->CurrentFileDirEntry;
	if(curFileDirEntry > 0){
		struct FileClass * fileInst = getFileClassFromList((curFileDirEntry - 1), lst);
		FILINFO finfo; 
		if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){
			libfatAttributes = (uint8)fatfs2libfatAttrib((int)finfo.fattrib);
		}
	}
	return libfatAttributes;
}

u8 FAT_SetFileAttributes (const char* filename, u8 attributes, u8 mask){
	u8	libfatAttributesIn = 0;
	u8	libfatAttributesOut= 0;
	struct FileClass fileInst;
	int sizeToCopy = 0;
	if(strlen(filename) > sizeof(fileInst.fd_namefullPath)){
		sizeToCopy = sizeof(fileInst.fd_namefullPath);
	}
	else{
		sizeToCopy = strlen(filename);
	}
	snprintf(fileInst.fd_namefullPath, sizeToCopy, "%s", filename);
	FILINFO finfo; 
	if(getFileFILINFOfromFileClass(&fileInst, &finfo) == true){	
		libfatAttributesIn = (uint8)fatfs2libfatAttrib((int)finfo.fattrib);
		libfatAttributesOut = (libfatAttributesIn & ~(mask & 0x27)) | (attributes & 0x27);
		int	NEWgccnewlibnano_to_fatfsAttributes = libfat2fatfsAttrib((int)libfatAttributesOut);
		int NEWmask = libfat2fatfsAttrib((int)mask);
		SetfatfsAttributesToFile((char*)filename, NEWgccnewlibnano_to_fatfsAttributes, NEWmask);
	}
	return libfatAttributesOut;
}

//Internal

//FAT_GetAlias
//Get the alias (short name) of the last file or directory entry read
//char* alias OUT: will be filled with the alias (short filename),
//	should be at least 13 bytes long
//bool return OUT: return true if successful
bool FAT_GetAlias(char* alias, struct FileClassList * lst){
	if (alias == NULL){
		return false;
	}
	int CurEntry = structfd_posixInvalidFileDirOrBufferHandle;
	
	int lastFileEntry = lst->LastFileEntry;
	int lastDirEntry = lst->LastDirEntry;
	
	if(lastFileEntry > lastDirEntry){
		CurEntry = lastFileEntry;
	}
	else{
		CurEntry = lastDirEntry;
	}
	//for some reason the CurEntry is invalid (trying to call and fileList hasn't been rebuilt)
	if(CurEntry == structfd_posixInvalidFileDirOrBufferHandle){
		return false;
	}
	struct FileClass * fileInst = getFileClassFromList(lst->CurrentFileDirEntry, lst);	//assign a FileClass to the StructFD generated before
	FILINFO finfo;
	if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){			//actually open the file and check attributes (rather than read dir contents)
		if (	 
		(	//file
		(finfo.fattrib & AM_RDO)
		||
		(finfo.fattrib & AM_HID)
		||
		(finfo.fattrib & AM_SYS)
		||
		(finfo.fattrib & AM_DIR)
		||
		(finfo.fattrib & AM_ARC)
		)
		||	//dir
		(finfo.fattrib & AM_DIR)
		)
		{
			strcpy((char*)alias,(const char *)fileInst->fd_namefullPath);	//update source path using short file/directory name				
		}
		//not file or dir
		else{
			return false;
		}
	}
	return true;
}

//stubbed because what these do is a workaround, described below:
//in TGDS: while listing a dir, create/read/update/delete a new file works
void FAT_preserveVars(){
}

void FAT_restoreVars(){
}

u32	disc_HostType(void){
	if(FS_InitStatus == true){
		return dldiGet()->ioInterface.ioType;
	}
	return 0;
}

/*-----------------------------------------------------------------
FAT_GetLongFilename
Get the long name of the last file or directory retrived with 
	GetDirEntry. Also works for FindFirstFile and FindNextFile.
	If a long name doesn't exist, it returns the short name
	instead.
char* filename: OUT will be filled with the filename, should be at
	least 256 bytes long
bool return OUT: return true if successful
-----------------------------------------------------------------*/
bool FAT_GetLongFilename(char* Longfilename, struct FileClassList * lst){
	if (Longfilename == NULL){
		return false;
	}
	int CurEntry = structfd_posixInvalidFileDirOrBufferHandle;
	if(lst->LastFileEntry > lst->LastDirEntry){
		CurEntry = lst->LastFileEntry;
	}
	else{
		CurEntry = lst->LastDirEntry;
	}
	//for some reason the CurEntry is invalid (trying to call and fileList hasn't been rebuilt)
	if(CurEntry == structfd_posixInvalidFileDirOrBufferHandle){
		return false;
	}
	struct FileClass * fileInst = getFileClassFromList(CurEntry, lst);	//assign a FileClass to the StructFD generated before
	FILINFO finfo;
	if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){			//actually open the file and check attributes (rather than read dir contents)
		if (	 
		(	//file
		(finfo.fattrib & AM_RDO)
		||
		(finfo.fattrib & AM_HID)
		||
		(finfo.fattrib & AM_SYS)
		||
		(finfo.fattrib & AM_DIR)
		||
		(finfo.fattrib & AM_ARC)
		)
		||	//dir
		(finfo.fattrib & AM_DIR)
		)
		{
			strcpy ((char*)Longfilename, (const char *)fileInst->fd_namefullPath);	//update source path using Long file/directory name
		}
		//not file or dir
		else{
			return false;
		}
	}
	return true;
}

/*-----------------------------------------------------------------
FAT_GetFileSize
Get the file size of the last file found or openned.
This idea is based on a modification by MoonLight
u32 return OUT: the file size
-----------------------------------------------------------------*/
u32 FAT_GetFileSize(struct FileClassList * lst){
	u32 fileSize = 0;
	struct FileClass * fileInst = getFileClassFromList(lst->LastFileEntry, lst);	//assign a FileClass to the StructFD generated before
	FILINFO finfo; 
	if(getFileFILINFOfromFileClass(fileInst, &finfo) == true){			//actually open the file and check attributes (rather than read dir contents)	
		if (//file
		(finfo.fattrib & AM_RDO)
		||
		(finfo.fattrib & AM_HID)
		||
		(finfo.fattrib & AM_SYS)
		||
		(finfo.fattrib & AM_DIR)
		||
		(finfo.fattrib & AM_ARC)
		)
		{
			fileSize = (u32)finfo.fsize;
		}
	}
	return 	fileSize;
}

/*-----------------------------------------------------------------
FAT_GetFileCluster
Get the first cluster of the last file found or openned.
u32 return OUT: the file start cluster
-----------------------------------------------------------------*/
u32 FAT_GetFileCluster(struct FileClassList * lst){
	u32 FirstClusterFromLastFileOpen = structfd_posixInvalidFileDirOrBufferHandle;
	struct FileClass * fileInst = getFileClassFromList(lst->LastFileEntry, lst);	//assign a FileClass to the StructFD generated before
	char * FullPathStr = fileInst->fd_namefullPath;	//must store proper filepath	must return fullPath here (0:/folder0/filename.ext)
	FILE * f = fopen(FullPathStr,"r");
	sint32 structFDIndex = structfd_posixInvalidFileDirOrBufferHandle;
	struct fd * fdinst = NULL;
	if(f){
		structFDIndex = fileno(f);
		fdinst = getStructFD(structFDIndex);
		FirstClusterFromLastFileOpen = getStructFDFirstCluster(fdinst);
		fclose(f);
	}
	return 	FirstClusterFromLastFileOpen;
}

/*-----------------------------------------------------------------
FAT_DisableWriting
Disables writing to the card at the driver level.
Cannot be re-enabled.
-----------------------------------------------------------------*/
bool disableWriting = false;
void FAT_DisableWriting (void){
	disableWriting = true;
}

/*-----------------------------------------------------------------
FAT_FileExists
Returns the type of file 
char* filename: IN filename of the file to look for
FILE_TYPE return: OUT returns FT_NONE if there is now file with 
	that name, FT_FILE if it is a file and FT_DIR if it is a directory
-----------------------------------------------------------------*/
int FAT_FileExists(char* filename){
	return FileExists(filename);	//assign a StructFD inside
}



/*-----------------------------------------------------------------
FAT_FreeFiles
Closes all open files then resets the CF card.
Call this before exiting back to the GBAMP
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_FreeFiles (void){
	char * devoptabFSName = (char*)"0:/";
	initTGDS(devoptabFSName);
	// Return status of card
	struct  DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)DLDIARM7Address;	
	return (bool)dldiInterface->ioInterface.isInserted();
}


/*-----------------------------------------------------------------
FAT_InitFiles
Reads the FAT information from the CF card.
You need to call this before reading any files.
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_InitFiles (void){
	return true;	//TGDS assumes the card was already inited if you followed the TGDS Standard ARM9 Init code start
}

/////////////////////////////////////////////Libfat wrapper layer End/////////////////////////////////////////////
//

bool setFileClass(bool iterable, char * fullPath, int FileClassListIndex, int Typ, int StructFD, struct FileClassList * lst){
	if( (lst != NULL) && (FileClassListIndex < FileClassItems) ){	//prevent overlapping current FileClassList 
		struct FileClass * FileClassInst = getFileClassFromList(FileClassListIndex, lst);
		FileClassInst->isIterable = iterable;
		strcpy(FileClassInst->fd_namefullPath, (const char *)fullPath);
		FileClassInst->type = Typ;
		FileClassInst->d_ino = StructFD;
		FileClassInst->curIndexInsideFileClassList = FileClassListIndex;
		FileClassInst->parentFileClassList = lst;	//Parent FileClass List node
		lst->FileDirCount++;
		return true;
	}
	return false;
}

bool setFileClassObj(int FileClassListIndex, struct FileClass * FileClassObj, struct FileClassList * lst){
	if( (lst != NULL) && (FileClassListIndex < FileClassItems) ){	//prevent overlapping current FileClassList		
		struct FileClass * thisFileClass = getFileClassFromList(FileClassListIndex, lst);
		memcpy(((u8*)thisFileClass + 0x400000), (u8*)FileClassObj, sizeof(struct FileClass));	//uncached
		lst->FileDirCount++;
		return true;
	}
	return false;
}

struct FileClass * getFileClassFromList(int FileClassListIndex, struct FileClassList * lst){
	return (struct FileClass *)&lst->fileList[FileClassListIndex];
}

//path can be either Directory or File, a proper FileClass will be returned.
struct FileClass getFileClassFromPath(char * path){
	struct FileClass FileClassOut;
	FileClassOut.type = FT_NONE;
	FILE * fh = fopen(path,"r");
	if(fh != NULL){
		FileClassOut.type = FT_FILE;
		fclose(fh);
	}
	else{
		//try dir
		DIR * dirHandle = opendir((const sint8 *)path);
		if(dirHandle != NULL){
			FileClassOut.type = FT_DIR;
			closedir(dirHandle);
		}
	}
	if((FileClassOut.type == FT_FILE)||(FileClassOut.type == FT_DIR)){
		strcpy(FileClassOut.fd_namefullPath, path);
	}
	else{
		strcpy(FileClassOut.fd_namefullPath, "");
	}
	
	FileClassOut.isIterable = false;	//destroyable FileClass item, does not belong to a list
	FileClassOut.d_ino = structfd_posixInvalidFileDirOrBufferHandle;	//destroyable FileClass are always invalid filehandles
	return FileClassOut;
}



//requires a previously generated struct fd *
bool getFileFILINFOfromFileClass(struct FileClass * fileInst, FILINFO * finfo){
	if(fileInst){
		FRESULT result = f_stat((const TCHAR*)fileInst->fd_namefullPath, finfo);					/* Get file status */
		if (result == FR_OK)
		{
			//stat ok
			return true;
		}
		else{
			//stat error
			errno = fresultToErrno(result);
		}
	}
	return false;
}

//Note: Requires a fresh call to buildFileClassListFromPath prior to calling this
struct FileClass * getFirstDirEntryFromList(struct FileClassList * lst){
	int i = 0;
	struct FileClass * FileClassRet = NULL;
	for(i = 0; i < FileClassItems; i++){
		struct FileClass * fileClassInst = getFileClassFromList(i, lst);
		if(fileClassInst->type == FT_DIR){
			FileClassRet = fileClassInst;
			break;
		}
	}
	lst->CurrentFileDirEntry = i;	//CurrentFileDirEntry is relative to getFirstDirEntryFromList() now
	return FileClassRet;
}

//Note: Requires a fresh call to buildFileClassListFromPath prior to calling this
struct FileClass * getFirstFileEntryFromList(struct FileClassList * lst){
	int i = 0;
	struct FileClass * FileClassRet = NULL;
	for(i = 0; i < FileClassItems; i++){
		struct FileClass * fileClassInst = getFileClassFromList(i, lst);
		if(fileClassInst->type == FT_FILE){
			FileClassRet = fileClassInst;
			break;
		}
	}
	lst->CurrentFileDirEntry = i;	//CurrentFileDirEntry is relative to getFirstDirEntryFromList() now
	return FileClassRet;
}

//returns: true if read success, otherwise error if false 
//filename_out must be at least MAX_TGDSFILENAME_LENGTH+1
bool readFileNameFromFileClassIndex(char* filename_out, struct FileClass * FileClassInst){
	if ((filename_out == NULL) || (FileClassInst == NULL)){
		return false;
	}
	char *  FullPathStr = FileClassInst->fd_namefullPath;
	strcpy(filename_out, FullPathStr);
	return true;
}

//returns:  Valid FileClass entry or NULL
struct FileClass * getFirstFile(char * path, struct FileClassList * lst, int startFromGivenIndex){
	struct FileClass * fileInst = NULL;
	//path will have the current working directory the FileClass was built around. getFirstFile builds everything, and getNextFile iterates over each file until there are no more.
	if(buildFileClassListFromPath(path, lst, startFromGivenIndex) == true){
		lst->CurrentFileDirEntry = startFromGivenIndex;
		fileInst = getFileClassFromList(lst->CurrentFileDirEntry, lst);
		int fType = fileInst->type;	//FT_NONE / FT_FILE / FT_DIR
		switch(fType){
			//dir
			case(FT_DIR):{
				lst->LastDirEntry=lst->CurrentFileDirEntry;
				readFileNameFromFileClassIndex((char*)path, fileInst);	//update source path
			}
			break;
			//file
			case(FT_FILE):{
				lst->LastFileEntry=lst->CurrentFileDirEntry;
				readFileNameFromFileClassIndex((char*)path, fileInst);	//update source path
			}			
			break;
			//empty / no dir or file
			default:{
				return NULL;	//End the list regardless, no more room available!. 
			}
			break;
		}
		//increase the file/dir counter after operation only if valid entry, otherwise it doesn't anymore
		if(lst->CurrentFileDirEntry < (int)(FileClassItems)){ 
			lst->CurrentFileDirEntry++;	
		}
		else{
			lst->CurrentFileDirEntry = 0;
			lst->LastDirEntry=structfd_posixInvalidFileDirOrBufferHandle;
			lst->LastFileEntry=structfd_posixInvalidFileDirOrBufferHandle;
			return NULL;	//End the list regardless, no more room available!
		}
	}
	return fileInst;
}

//requires fullpath of the CURRENT file, it will return the next one
//return:  Valid FileClass entry or NULL
struct FileClass * getNextFile(char * path, struct FileClassList * lst){
	struct FileClass * fileInst = getFileClassFromList(lst->CurrentFileDirEntry, lst);	//NULL == invalid. Should not happen 
	if(fileInst != NULL){
		int fType = fileInst->type;
		switch(fType){
			//dir
			case(FT_DIR):{
				lst->LastDirEntry=lst->CurrentFileDirEntry;
				readFileNameFromFileClassIndex((char*)path, fileInst);	//update source path
			}
			break;
			//file
			case(FT_FILE):{
				lst->LastFileEntry=lst->CurrentFileDirEntry;
				readFileNameFromFileClassIndex((char*)path, fileInst);	//update source path
			}
			break;
		}	
		//increase the file counter after operation
		if(lst->CurrentFileDirEntry < (int)(FileClassItems)){ 
			lst->CurrentFileDirEntry++;	
		}
		else{
			lst->CurrentFileDirEntry = 0;
			lst->LastDirEntry=structfd_posixInvalidFileDirOrBufferHandle;
			lst->LastFileEntry=structfd_posixInvalidFileDirOrBufferHandle;
			return NULL;	//End the list regardless, no more room available!
		}
	}
	return fileInst;
}

//TGDS FS Directory Iterator object:
//returns current count of such context.
//Otherwise error (structfd_FileClassListInvalidEntry) if: 
//	- FileClassList exceeded the FileClass items to store
//	- FileClassList exceeded the FileClass items to store

int getCurrentDirectoryCount(struct FileClassList * lst){
	if(lst != NULL){
		return lst->FileDirCount;
	}
	return structfd_FileClassListInvalidEntry;
}

void setCurrentDirectoryCount(struct FileClassList * lst, int value){
	lst->FileDirCount = value;
} 
///////////////////////////////////////////////////////TGDS FS API extension end. /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////USER CODE END/////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////INTERNAL CODE START///////////////////////////////////////////////////////////////////////////////
int  readdir_r(DIR * dirp,struct dirent * entry,struct dirent ** result){
    return fatfs_readdir_r(dirp, entry, result);
}

int fatfs_write(int structFDIndex, sint8 *ptr, int len){	//(FileDescriptor :struct fd index)
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    struct fd *pfd = getStructFD(structFDIndex);
    if (pfd == NULL){	//not file? not alloced struct fd?
		errno = EBADF;
    }
    else if (
	(pfd->filPtr == NULL)	//no FIL descriptor?
	&&
	(pfd->dirPtr == NULL)	//and no DIR descriptor? definitely can't write to this file descriptor
	){
        errno = EBADF;
    }
    else if (S_ISREG(pfd->stat.st_mode)){
        FIL *filp;
        FRESULT result;
        UINT written;
        filp = pfd->filPtr;
        if (pfd->status_flags & O_APPEND){
            DWORD size;
            size = f_size(filp);
            result = f_lseek(filp, size);
        }
		else if(filp == NULL){
			result = FR_INVALID_OBJECT;
		}
        else{
            result = FR_OK;
        }
        if (result == FR_OK){
            result = f_write(filp, ptr, len, &written);	//writes success: if ret directory file invalid (7) check permission flags passed through fopen
        }
        if (result == FR_OK){
            ret = written;
        }
        else{
            errno = fresultToErrno(result);
        }
    }
    else if (S_ISDIR(pfd->stat.st_mode)){
		errno = EISDIR;
    }
    else{
        errno = EBADF;
    }
    return ret;
}

//read (get struct FD index from FILE * handle)
int fatfs_read(int structFDIndex, sint8 *ptr, int len){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    struct fd *pfd = getStructFD(structFDIndex);
    if ( (pfd == NULL) || (pfd->filPtr == NULL) ){	//not file/dir? not alloced struct fd?
        errno = EBADF;
    }
    else if (S_ISREG(pfd->stat.st_mode)){
        FIL *filp;
        FRESULT result;
        UINT nbytes_read;
        filp = pfd->filPtr;
        result = f_read(filp, ptr, len, &nbytes_read);
        if (result == FR_OK){
            ret = nbytes_read;
        }
        else{
            errno = fresultToErrno(result);
        }
    }
    else if (S_ISDIR(pfd->stat.st_mode)){
        errno = EISDIR;
	}
    else{
        errno = EBADF;
    }
    return ret;
}

//receives a new struct fd index with either DIR or FIL structure allocated so it can be closed.
//returns 0 if success, 1 if error
int fatfs_close (int structFDIndex){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    struct fd * pfd = getStructFD(structFDIndex);
    if ( (pfd == NULL) || ((pfd->filPtr == NULL) && (pfd->dirPtr == NULL)) || (pfd->isused != (sint32)structfd_isused) || (structFDIndex == structfd_posixInvalidFileDirOrBufferHandle) ){	//not file/dir? not alloced struct fd? or not valid file handle(two cases)?
		errno = EBADF;
    }
	//File?
    else if (S_ISREG(pfd->stat.st_mode)){
        FIL *filp;
        FRESULT result;
        filp = pfd->filPtr;
        result = f_close(filp);
        if (
		(result == FR_OK)			//file sync with hardware SD ok
		||
		(result == FR_DISK_ERR)		//create file (fwrite + w): file didn't exist before open(); thus file descriptor didn't have any data of sectors to compare with. Exception expected
		){
			//struct stat buf;	//flush stat
			memset (&pfd->stat, 0, sizeof(pfd->stat));
			
			//free struct fd
			int i_fil = FileHandleFree(pfd->cur_entry.d_ino);	//returns structfd index that was deallocated
			if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
				if(pfd->filPtr){	//must we clean a FIL?
					pfd->filPtr = NULL;
				}
				if(pfd->dirPtr){	//must we clean a DIR?
					pfd->dirPtr = NULL;
				}
				//clean filename
				sprintf((char*)&pfd->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
			}
			else{
				//file_free failed
			}
			
			ret = 0;
			//update d_ino here (POSIX compliant)
			pfd->cur_entry.d_ino = (sint32)structfd_posixInvalidFileDirOrBufferHandle;
			pfd->loc = 0;
        }
        else{
			errno = fresultToErrno(result);
        }
    }
	//Directory?
    else if (S_ISDIR(pfd->stat.st_mode)){
        DIR *dp;
        FRESULT result;
		dp = pfd->dirPtr;
        result = f_closedir(dp);
        if (result == FR_OK){
			FileHandleFree(pfd->cur_entry.d_ino);
            ret = 0;
			pfd->loc = 0;
        }
        else{
			errno = fresultToErrno(result);
        }
    }
    else{
		errno = EBADF;
    }
	return ret;
}

int fatfs_fsize (int structFDIndex){
	struct fd * structfd = getStructFD(structFDIndex);
	if(structfd != NULL){
		return f_size(structfd->filPtr);
	}
	return structfd_posixInvalidFileSize;
}

void fillPosixStatStruct(const FILINFO *fno, struct stat *out){
    mode_t mode;
    memset(out, 0, sizeof(struct stat));
    out->st_size = fno->fsize;
    if ((fno->fattrib & AM_MASK) & AM_DIR){
        mode = S_IFDIR;
    }
    else{
        mode = S_IFREG;
    }
    mode |= (S_IRUSR|S_IRGRP|S_IROTH);
    mode |= (S_IXUSR|S_IXGRP|S_IXOTH);
    if (!((fno->fattrib & AM_MASK) & AM_RDO)){
        /* rwxrwxrwx */
        mode |= (S_IWUSR|S_IWGRP|S_IWOTH);
    }
    else{
        /* r-xr-xr-x */
    }
    out->st_mode = mode;
	
#if 0
    /* not present in newlib struct stat */
    struct timespec ts;
    fattime_to_timespec(fno->fdate, fno->ftime, &ts);
    pfd->stat.st_atim = ts;
    pfd->stat.st_mtim = ts;
    pfd->stat.st_ctim = ts;
#endif
}

//called from : stat (newlib implementation)
void initStructFDHandle(struct fd *pfd, int flags, const FILINFO *fno, int structFD, int StructFDType){
    pfd->isatty = 0;
    pfd->status_flags = flags;
    pfd->descriptor_flags = 0;
	pfd->loc = 0;	//internal file/dir offset zero
	pfd->StructFD = structFD;
	pfd->StructFDType = StructFDType;
	fillPosixStatStruct(fno, &pfd->stat);
}

//returns an internal index struct fd allocated
int fatfs_open_file(const sint8 *pathname, int flags, const FILINFO *fno){
	//Lookup if file is already open.
	int structFDIndex = getStructFDIndexByFileName((char*)pathname);
	if(structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		return structFDIndex;
	}
	//If not, then allocate a new file handle (struct FD)
	BYTE mode;
	FRESULT result;
	
	//allocates a new struct fd index, allocating a FIL structure, for the devoptab_sdFilesystem object.
	structFDIndex = FileHandleAlloc((struct devoptab_t *)&devoptab_sdFilesystem);	
    if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		(files + structFDIndex)->filPtr	=	(FIL *)&(files + structFDIndex)->fil;
		(files + structFDIndex)->dirPtr	= NULL;
		(files + structFDIndex)->StructFDType = FT_FILE;
	}
	
	struct fd * fdinst = getStructFD(structFDIndex);	//getStructFD requires struct fd index
	if (fdinst == NULL){
		result = FR_TOO_MANY_OPEN_FILES;
    }
	else{
		FILINFO fno_after;
        if(flags & O_CREAT){
			result = f_unlink(pathname);
			if (result == FR_OK){
				//file was deleted
			}
			else{
				//file doesn´t exist
			}
		}
		mode = posixToFatfsAttrib(flags);
		result = f_open(fdinst->filPtr, pathname, mode);	/* Opens an existing file. If not exist, creates a new file. */
		if (result == FR_OK){		
			result = f_stat(pathname, &fno_after);
			fno = &fno_after;			
			if ((result == FR_OK) && (TGDSFS_detectUnicode(fdinst) == true)){
				//Update struct fd with new FIL
				initStructFDHandle(fdinst, flags, fno, structFDIndex, FT_FILE);
				//copy full file path (posix <- fatfs)
				int topsize = strlen(pathname)+1;
				if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
					topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
				}
				strncpy(fdinst->fd_name, pathname, topsize);
			}
			else{
				errno = fresultToErrno(result);
				
				//free struct fd
				int i_fil = FileHandleFree(fdinst->cur_entry.d_ino);	//returns structfd index that was deallocated
				if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
					if(fdinst->filPtr){	//must we clean a FIL?
						fdinst->filPtr = NULL;
					}
					if(fdinst->dirPtr){	//must we clean a DIR?
						fdinst->dirPtr = NULL;
					}
					//clean filename
					sprintf((char*)&fdinst->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
				}
				else{
					//file_free failed
				}
			
				structFDIndex = structfd_posixInvalidFileDirOrBufferHandle;	//file stat was valid but something happened while IO operation, so, invalid.
			}
		}
        else {	//invalid file or O_CREAT wasn't issued
			
			//free struct fd
			int i_fil = FileHandleFree(fdinst->cur_entry.d_ino);	//returns structfd index that was deallocated
			if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
				if(fdinst->filPtr){	//must we clean a FIL?
					fdinst->filPtr = NULL;
				}
				if(fdinst->dirPtr){	//must we clean a DIR?
					fdinst->dirPtr = NULL;
				}
				//clean filename
				sprintf((char*)&fdinst->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
			}
			else{
				//file_free failed
			}
			structFDIndex = structfd_posixInvalidFileDirOrBufferHandle;	//file handle generated, but file open failed, so, invalid.
		}
    }// failed to allocate a file handle / allocated file handle OK end.
	
	return structFDIndex;
}

//returns an internal index struct fd allocated
int fatfs_open_dir(const sint8 *pathname, int flags, const FILINFO *fno){
    FRESULT result;
    
	//allocates a new struct fd index, allocating a DIR structure, for the devoptab_sdFilesystem object.
	int structFDIndex = FileHandleAlloc((struct devoptab_t *)&devoptab_sdFilesystem);	
    if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		(files + structFDIndex)->dirPtr	=	(DIR *)&(files + structFDIndex)->dir;
		(files + structFDIndex)->filPtr	= NULL;
		(files + structFDIndex)->StructFDType = FT_DIR;
	}
	
	struct fd * fdinst = getStructFD(structFDIndex);
	if (fdinst == NULL){
		result = FR_TOO_MANY_OPEN_FILES;
    }
    else{
		result = f_opendir(fdinst->dirPtr, pathname);
        if (result == FR_OK){
			//Update struct fd with new DIR
			initStructFDHandle(fdinst, flags, fno, structFDIndex, FT_DIR);
			//copy full directory path (posix <- fatfs)
			int topsize = strlen(pathname)+1;
			if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
				topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
			}
			strncpy(fdinst->fd_name, pathname, topsize);
        }
        else{
			errno = fresultToErrno(result);            
			//free struct fd
			int i_fil = FileHandleFree(fdinst->cur_entry.d_ino);	//returns structfd index that was deallocated
			if (i_fil != structfd_posixInvalidFileDirOrBufferHandle){	//FileHandleFree could free struct fd properly? set filesAlloc[index] free
				if(fdinst->filPtr){	//must we clean a FIL?
					fdinst->filPtr = NULL;
				}
				if(fdinst->dirPtr){	//must we clean a DIR?
					fdinst->dirPtr = NULL;
				}
				//clean filename
				sprintf((char*)&fdinst->fd_name[0],"%s",(char*)&devoptab_stub.name[0]);
			}
			else{
				//file_free failed
			}
        }
    }

    return structFDIndex;
}

//returns / allocates a new struct fd index with either DIR or FIL structure allocated
int fatfs_open_file_or_dir(const sint8 *pathname, int flags){
	FILINFO fno;
	int structFD = structfd_posixInvalidFileDirOrBufferHandle;
	fno.fname[0] = '\0'; /* initialize as invalid */
	FRESULT result = f_stat(pathname, &fno);
	//dir case // todo: same logic as below file if dir does not exists and must be created
    if ((result == FR_OK) && ((fno.fattrib & AM_MASK) & AM_DIR)){
        structFD = fatfs_open_dir(pathname, flags, &fno);
	}
	else if (
		(result == FR_OK)	//file exists case
		||
		(flags & O_CREAT)	//new file?
	){
        structFD = fatfs_open_file(pathname, flags, &fno);	//returns / allocates a new struct fd index with either DIR or FIL structure allocated
	}
    else {
		//file/dir does not exist, couldn't create
		errno = fresultToErrno(result);
	}
    return structFD;
}

// Use NDS RTC to update timestamps into files when certain operations require it
DWORD get_fattime (void){
	struct tm * tmStruct = getTime();
    return ((((sint32)tmStruct->tm_year-60)<<25)
				|
				(((sint32)tmStruct->tm_mon)<<21)
				|
				(((sint32)tmStruct->tm_mday)<<16)
				|
				(((sint32)tmStruct->tm_hour)<<11)
				|
				(((sint32)tmStruct->tm_min)<<5)
				|
				(((sint32)tmStruct->tm_sec)<<0)	);
}

//returns / allocates a new struct fd index with either DIR or FIL structure allocated
//if error, returns structfd_posixInvalidFileDirOrBufferHandle (invalid structFD file handle index)
int fatfs_open(const sint8 *pathname, int flags){
	int structFDIndex = fatfs_open_file_or_dir(pathname, flags);	
	if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		//update d_ino here (POSIX compliant) - through correct (internal struct fd) file handle
		struct fd *pfd = getStructFD(structFDIndex);
		pfd->cur_entry.d_ino = structFDIndex;
	}
	return structFDIndex;
}
// ret: structfd_posixInvalidFileDirOrBufferHandle if invalid posixFDStruct
//		else if POSIX retcodes (if an error happened)
//		else return new offset position (offset + current file position (internal)) in file handle
off_t fatfs_lseek(int structFDIndex, off_t offset, int whence){	//(FileDescriptor :struct fd index)
    off_t ret = (off_t)structfd_posixInvalidFileDirOrBufferHandle;
    struct fd *pfd = getStructFD(structFDIndex);
    if ((pfd == NULL) || (pfd->filPtr == NULL) || (pfd->isused == structfd_isunused) ){
		errno = EBADF;
    }
	else if (S_ISSOCK(pfd->stat.st_mode)){	//socket? cant use
		ret = EPIPE;
	}
    else if (S_ISREG(pfd->stat.st_mode)){	//only file allows this
        FIL * filp = pfd->filPtr;
        FRESULT result;
        DWORD pos = 0;
        int topFile = f_size(filp);
		bool validArg = false;
		switch(whence&0x3){
			//<<SEEK_SET>>---<[offset]> is the absolute file position (an offset
			//from the beginning of the file) desired.  <[offset]> must be positive.
			case(SEEK_SET):{
				if(offset < 0){
					offset = 0;
				}
				if(offset >= topFile){
					offset = (topFile - 1);	//offset starts from 0 so -1 here
				}
				pos = offset;
				validArg = true;
			}
			break;
			//<<SEEK_CUR>>---<[offset]> is relative to the current file position.
			//<[offset]> can meaningfully be either positive or negative.
			case(SEEK_CUR):{
				pos = f_tell(filp);
				pos += offset;
				if((int)pos < 0){
					pos = 0;
				}
				if(pos >= topFile){
					pos = (topFile - 1);	//offset starts from 0 so -1 here
				}
				validArg = true;
			}
			break;
			//<<SEEK_END>>---<[offset]> is relative to the current end of file.
			//<[offset]> can meaningfully be either positive (to increase the size
			//of the file) or negative.
			case(SEEK_END):{
				pos = topFile;				//file end is fileSize
				validArg = true;
			}
			break;	
		}
		if(validArg == true){
			result = f_lseek(filp, pos);
			if (result == FR_OK){
				ret = pos;
				//update stat st here
				
				//update current offset
				pfd->loc = pos;
			}
			else{
				errno = fresultToErrno(result);
			}
		}
	}
    else{
        errno = EINVAL;
    }
    return ret;
}

//delete / remove
int fatfs_unlink(const sint8 *path){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    FRESULT result;
    result = f_unlink(path);
    if (result == FR_OK){
        ret = 0;
    }
    else{
        errno = fresultToErrno(result);
    }
    return ret;
}

int fatfs_link(const sint8 *path1, const sint8 *path2){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    // FAT does not support links
    errno = EMLINK; /* maximum number of "links" is 1: the file itself */
    return ret;
}

int fatfs_rename(const sint8 *oldfname, const sint8 * newfname){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    FRESULT result;
    result = f_rename(oldfname, newfname);
    if (result != FR_OK){
        errno = fresultToErrno(result);
    }
    else{
        ret = 0;
    }
    return ret;
}

int fatfs_fsync(int structFDIndex){	//uses struct fd indexing
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    struct fd * pfd = getStructFD(structFDIndex);
    if (pfd == NULL){
        errno = EBADF;
    }
    else if (pfd->filPtr == NULL){
        errno = EBADF;
    }
    else if (S_ISREG(pfd->stat.st_mode)){
        FIL *filp = pfd->filPtr;
        FRESULT result = f_sync(filp);
        if (result == FR_OK){
            ret = 0;
        }
        else{
            errno = fresultToErrno(result);
        }
    }
    else{
        errno = EINVAL;
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirOrBufferHandle / struct stat * is empty or invalid
int fatfs_stat(const sint8 *path, struct stat *buf){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    FILINFO fno;
	FRESULT result = f_stat(path, &fno);
    if ((result == FR_OK) && (buf != NULL)){
        fillPosixStatStruct(&fno, buf);
        ret = 0;
    }
    else{
        errno = fresultToErrno(result);
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirOrBufferHandle if path exists or failed due to other IO reason
int fatfs_mkdir(const sint8 *path, mode_t mode){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    FRESULT result = f_mkdir(path);
    if (result == FR_OK){
        ret = 0;
    }
    else{
        errno = fresultToErrno(result);
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirOrBufferHandle if readonly or not empty, or failed due to other IO reason
int fatfs_rmdir(const sint8 *path){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    FRESULT result = f_unlink(path);
    if (result == FR_OK){
        ret = 0;
    }
    else if (result == FR_DENIED){
        FILINFO fno;
        // the dir was readonly or not empty: check 
        result = f_stat(path, &fno);
        if (result == FR_OK){
            if ((fno.fattrib & AM_MASK) & AM_RDO){
                errno = EACCES;
            }
            else{
                errno = ENOTEMPTY;
            }
        }
        else{
            errno = fresultToErrno(result);
        }
    }
    else{
        errno = fresultToErrno(result);
    }
    return ret;
}

//if ok: return 0
//else if error return structfd_posixInvalidFileDirOrBufferHandle if incorrect path, or failed due to other IO reason
int fatfs_chdir(const sint8 *path){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
    FRESULT result = f_chdir(path);
    if (result == FR_OK){
        ret = 0;
    }
    else{
        errno = fresultToErrno(result);   
    }
    return ret;
}

//if ok: return char * cwf path
//else if error return NULL if path is incorrect, or failed due to other IO reason
sint8 *fatfs_getcwd(sint8 *buf, size_t size){
	sint8 *ret = NULL;
    FRESULT result = f_getcwd(buf, size);
    if (result == FR_OK){
        ret = buf;
    }
    else{
        errno = fresultToErrno(result);
    }
    return ret;
}

//if correct path: returns: iterable DIR * entry
//else returns NULL
DIR *fatfs_opendir(const sint8 *path){
    DIR *ret = NULL;
    int structFDIndex = fatfs_open(path, O_RDONLY);	//returns an internal index struct fd allocated
	if (structFDIndex != structfd_posixInvalidFileDirOrBufferHandle){
		struct fd *pfd = getStructFD(structFDIndex);
        if (pfd != NULL){
            ret = pfd->dirPtr;
        }
        else{
			errno = EBADF;
        }
    }
    //else if: Invalid Path
    return ret;
}

//if the iterable DIR * was closed, return 0
//else returns structfd_posixInvalidFileDirOrBufferHandle if incorrect iterable DIR *
int fatfs_closedir(DIR *dirp){
	//we need a conversion from DIR * to struct fd *
	int structFDIndex = getStructFDIndexByDIR(dirp);
	return fatfs_close(structFDIndex);	//requires a struct fd(file descriptor), returns 0 if success, structfd_posixInvalidFileDirOrBufferHandle if error
}

//if iterable DIR * is directory, return 0
//else return structfd_posixInvalidFileDirOrBufferHandle (not valid iterable DIR * entry or not directory )
int fatfs_dirfd(DIR *dirp){
    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	//we need a conversion from DIR * to struct fd *
	int structFDIndex = getStructFDIndexByDIR(dirp);
	struct fd * pfd = getStructFD(structFDIndex);
    if ((pfd != NULL) && (S_ISDIR(pfd->stat.st_mode))){
		ret = 0;
    }
    else{
        errno = EINVAL;
    }
    return ret;
}

//if iterable DIR * is in the current StructFD index, return iterable DIR *
//else return NULL (not valid iterable DIR * entry or not directory )
DIR *fatfs_fdopendir(int structFDIndex){	//(FileDescriptor :struct fd index)
    DIR *ret = NULL;
    struct fd *pfd = getStructFD(structFDIndex);
    if (pfd == NULL){
        errno = EBADF;
    }
    else if (S_ISDIR(pfd->stat.st_mode)){
        ret = pfd->dirPtr;
    }
    else{
        errno = ENOTDIR;
    }
    return ret;
}

//if iterable DIR * has a struct dirent *(StructFD index, being an object), return such struct dirent * (internal fatfs file handle descriptor)
//else return NULL (not valid iterable DIR * entry or not directory )
struct dirent *fatfs_readdir(DIR *dirp){
    struct dirent *ret = NULL;
    int structFDIndex = getStructFDIndexByDIR(dirp);	//we need a conversion from DIR * to struct fd *
	struct fd * fdinst = getStructFD(structFDIndex);
	if(fatfs_readdir_r(dirp, (struct dirent *)&fdinst->cur_entry, &ret) != structfd_posixInvalidFileDirOrBufferHandle){
		return ret;
	}
	else{
        errno = EBADF;
	}
	//ret dirent * with read file/dir properties, namely: d_name
    return ret;
}

//if return equals 0 : means the struct dirent * (internal read file / dir) coming from DIR * was correctly parsed. 
//else if return equals structfd_posixInvalidFileDirOrBufferHandle: the DIR * could not parse the output struct dirent * (internal read file / dir), being NULL.
int fatfs_readdir_r(
        DIR *dirp,
        struct dirent *entry,	//current dirent of file handle struct fd
        struct dirent **result){	//pointer to rewrite above file handle struct fd

    int ret = structfd_posixInvalidFileDirOrBufferHandle;
	int structFDIndex = getStructFDIndexByDIR(dirp);
	struct fd * fdinst = getStructFD(structFDIndex);
	if(fdinst != NULL){
		FRESULT fresult;
		FILINFO fno;
		fresult = f_readdir(dirp, &fno);
		if (fresult != FR_OK){
			errno = fresultToErrno(fresult);
			*result = NULL;
		}
		// end of entries
		else if (fno.fname[0] == '\0'){
			ret = 0;
			*result = NULL;
		}
		else{
			ret = 0;
			
			//fill dir/file stats
			fdinst->loc++;
			
			//d_name : dir or file name. NOT full path (posix <- fatfs)
			int topsize = strlen(fno.fname)+1;
			if((sint32)topsize > (sint32)(MAX_TGDSFILENAME_LENGTH+1)){
				topsize = (sint32)(MAX_TGDSFILENAME_LENGTH+1);
			}
			strncpy((sint8*)entry->d_name, (sint8*)fno.fname, topsize);
			*result = entry;
			
			//update FT_FILE / FT_DIR
			if (fno.fattrib & AM_DIR) {
				fdinst->StructFDType = FT_DIR;
			}
			
			if (fno.fattrib & AM_ARC) {
				fdinst->StructFDType = FT_FILE;
			}
		}
    }
	return ret;
}

void fatfs_rewinddir(DIR *dirp){
	int structFDIndex = getStructFDIndexByDIR(dirp);
	struct fd * fdinst = getStructFD(structFDIndex);
	FRESULT result = f_readdir(dirp, NULL);
	if (result == FR_OK){
		fdinst->loc	= 0;
	}
	else if (S_ISDIR(fdinst->stat.st_mode)){
		//Not Directory!	/* POSIX says no errors are defined */
    }
	else{
		//Error handling the request.	/* POSIX says no errors are defined */
	}
}

long fatfs_tell(struct fd *f){	//NULL check already outside
    long ret = structfd_posixInvalidFileDirOrBufferHandle;
	//dir
	if ((f->dirPtr) && (S_ISDIR(f->stat.st_mode))){
		ret = f->loc;
    }
	//file
	else if ((f->filPtr) && (S_ISREG(f->stat.st_mode))){
		ret = f->loc;
	}
    return ret;
}

void fatfs_seekdir(DIR *dirp, long loc){
	int structFDIndex = getStructFDIndexByDIR(dirp);
	struct fd * fdinst = getStructFD(structFDIndex);
    if (S_ISDIR(fdinst->stat.st_mode)){
        long cur_loc = fatfs_tell(fdinst);
        if (loc < cur_loc){
            fatfs_rewinddir(dirp);
        }
        while(loc > cur_loc){
            int ret = structfd_posixInvalidFileDirOrBufferHandle;
            struct dirent entry;
            struct dirent *result = NULL;
            ret = fatfs_readdir_r(dirp, &entry, &result);
            if ((result == NULL) || (ret != 0))
            {
                /* POSIX says no errors are defined */
                break;
            }
            cur_loc = fatfs_tell(fdinst);
        }
    }
    else{
        /* POSIX says no errors are defined */
    }
}

//internal: SD init code: call fatfs_init() to have TGDS Filesystem support (posix file functions fopen/fread/fwrite/fclose/etc working). Always call first.
int fatfs_init(){
	char * devoptabFSName = (char*)"0:/";
	initTGDS(devoptabFSName);
    return (f_mount(&dldiFs, "0:", 1));
}

//internal: SD de-init code: requires to call fatfs_init() at least once before.
int fatfs_deinit(){
	int ret = f_unmount("0:");
	
	//ARM7 DLDI implementation
	#ifdef ARM7_DLDI
	ARM9DeinitDLDI();
	#endif
	
	//ARM9 DLDI impl.
	#ifdef ARM9_DLDI
	struct  DLDI_INTERFACE* dldiInterface = (struct DLDI_INTERFACE*)DLDIARM7Address;	
	dldiInterface->ioInterface.clearStatus();
	dldiInterface->ioInterface.shutdown();
	#endif
	
	return ret;
}

//this copies stat from internal struct fd to output stat
//if StructFD (structFDIndex) index is created by fatfs_open (either file / dir), return 0 and the such struct stat * is updated.
//else return structfd_posixInvalidFileDirOrBufferHandle (not valid struct stat * entry, being NULL)
int _fstat_r( struct _reent *_r, int structFDIndex, struct stat *buf ){	//(FileDescriptor :struct fd index)
    struct fd * f = getStructFD(structFDIndex);
    if ((f == NULL) || (f->isused == structfd_isunused)){
        _r->_errno = EBADF;
		return structfd_posixInvalidFileDirOrBufferHandle;
    }
    *buf = f->stat;
    return 0;
}


//true: able to detect if file was unicode or not
//false: couldn't tell if file was unicode or not
bool TGDSFS_detectUnicode(struct fd *pfd){
	if(pfd != NULL){
		FILE * fil = fdopen(pfd->cur_entry.d_ino, "r");
		if(fil != NULL){
			char tStr[4];
			u32 cPos = ftell(fil);
			fseek(fil, 0, SEEK_SET);
			fread((uint8*)&tStr[0], 1, 4, fil);
			if(tStr[0] == 0 && tStr[2] == 0 && tStr[1] != 0 && tStr[3] != 0){	// fairly good chance it's unicode
				pfd->fileIsUnicode = true;
			}
			else{
				pfd->fileIsUnicode = false;
			}
			fseek(fil, cPos, SEEK_SET);
			return true;
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////INTERNAL CODE END///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS /////////////////////////////////////////////////////////////////////

//returns the first free StructFD
bool buildFileClassListFromPath(char * path, struct FileClassList * lst, int startFromGivenIndex){
	//Decide wether we have a Working directory or not, if valid dir, enter that dir. If not, use the default dir and point to it. + Clear TGDS FS Directory Iterator context
	if((cleanFileList(lst) == true) && (updateTGDSFSDirectoryIteratorCWD(path, lst) == true) && (chdir(path) == 0) ){
		//rebuild filelist
		FRESULT result;
		DIR dir;
		int i = 0;
		FILINFO fno;
		result = f_opendir(&dir, path);                       /* Open the directory */
		if (result == FR_OK) {
			for(;;){
				result = f_readdir(&dir, &fno);                   /* Read a directory item */
				int type = 0;
				if (fno.fattrib & AM_DIR) {			           /* It is a directory */
					type = FT_DIR;	
				}
				else if (									   /* It is a file */
				(fno.fattrib & AM_RDO)
				||
				(fno.fattrib & AM_HID)
				||
				(fno.fattrib & AM_SYS)
				||
				(fno.fattrib & AM_ARC)
				){
					type = FT_FILE;			
				}
				else{	/* It is Invalid. */
					type = FT_NONE;
				}
				
				if (result != FR_OK || fno.fname[0] == 0){	//Error or end of dir. No need to handle the error here.
					break;
				}
				else if(i >= FileClassItems){
					break;
				}
				else if((type == FT_FILE) || (type == FT_DIR)){
					char builtFilePath[MAX_TGDSFILENAME_LENGTH+1];
					if(type == FT_DIR){
						sprintf(builtFilePath,"%s%s%s",path,"/",fno.fname);
					}
					else if(type == FT_FILE){
						sprintf(builtFilePath,"%s%s%s",getfatfsPath((sint8*)path),"/",fno.fname);
					}		
					//Generate FileClass from either a Directory or a File
					bool iterable = true;
					setFileClass(iterable, (char*)&builtFilePath[0], startFromGivenIndex + i, type, structfd_posixInvalidFileDirOrBufferHandle, lst);
					i++;
				}			
			}
			f_closedir(&dir);
			lst->FileDirCount = i;
		}
		else{
			errno = fresultToErrno(result);
			return false;
		}
		return true;
	}
	return false;
}

//Directory Functions
bool enterDir(char* newDir, char* destinationPath){
	if((newDir != NULL) && (destinationPath != NULL)){
		strcpy(destinationPath, newDir); //update target DIR
		return true;
	}
	return false;
}

//passes the full working directory held by the CWD from within this context is called from, then, 
//removes the last directory of such context 
bool leaveDir(char* oldNewDir){
	if(oldNewDir != NULL){
		char tempnewDir[MAX_TGDSFILENAME_LENGTH+1] = {0};
		char tempnewDiroutPath[MAX_TGDSFILENAME_LENGTH+1] = {0};    //used by splitCustom function as output path buffer
		strcpy(tempnewDir, (const char *)oldNewDir);
		getLastDirFromPath(tempnewDir, TGDSDirectorySeparator, tempnewDiroutPath);
		strcpy(oldNewDir, tempnewDiroutPath);
		return true;
	}
	return false;
}

//Global Current Working Directory updated by the FileClass iterator. char * path : If a valid directory is passed, it will be used to populate the FileClassList. Otherwise the default Start Directory is used.
//Note: 
//Step 1): The Global CWD will be overriden by the char * path passed
//Step 2): If any File or Directory is found then char * path will be destroyed by the current iterated File / Directory item.
bool updateTGDSFSDirectoryIteratorCWD(char * newCWD, struct FileClassList * lst){
	char * CurrentWorkingDirectory = (char*)&lst->TGDSCurrentWorkingDirectory[0];
	if(strlen(newCWD) > 0){
		strcpy(CurrentWorkingDirectory, (const char*)newCWD);
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////// INTERNAL DIRECTORY FUNCTIONS END //////////////////////////////////////////////////////////////////
