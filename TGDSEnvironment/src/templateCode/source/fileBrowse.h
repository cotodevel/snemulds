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

#ifndef __fileBrowse_h__
#define __fileBrowse_h__

#include "typedefsTGDS.h"
#include "dsregs.h"
#include "keypadTGDS.h"
#include "InterruptsARMCores_h.h"

static inline void parseDirNameTGDS(char * dirName){
	int dirlen = strlen(dirName);
	if(dirlen > 2){
	    int i = 0;
		
		//trim the starting / if it has one
		if ( (dirName[0] == '/') && (dirName[1] == '/') ) {
			char fixDir[MAX_TGDSFILENAME_LENGTH+1];
		    strcpy(fixDir, (char*)&dirName[1]);
			strcpy(dirName, "");	//clean
            strcpy(dirName, fixDir);
		}
		char tempDir[MAX_TGDSFILENAME_LENGTH+1];
		strcpy(tempDir, dirName);
		
		if(tempDir[strlen(tempDir)-1]== '/'){
		   tempDir[strlen(tempDir)-1] = '\0'; 
		}
		strcpy(dirName, "");	//clean
		strcpy(dirName, tempDir);
	}
	
}

static inline void parsefileNameTGDS(char * fileName){
	int filelen = strlen(fileName) + 1;
	if(filelen > 4){
	    if (fileName[0] == '/') {
			char fixDir[MAX_TGDSFILENAME_LENGTH+1];       //trim the starting / if it has one
		    memset(fixDir, 0, sizeof(fixDir));
		    strcpy(fixDir, (char*)&fileName[1]);
		    memset(fileName, 0, strlen(fileName));
            strcat(fileName, fixDir);
		}
		if ((fileName[2] == '/') && (fileName[3] == '/')) {
		    char fixDir[MAX_TGDSFILENAME_LENGTH+1];       //trim the starting // if it has one (since getfspath appends 0:/)
		    memset(fixDir, 0, strlen(fixDir));
		    fixDir[0] = fileName[0];
		    fixDir[1] = fileName[1];
		    fixDir[2] = fileName[2];
		    strcat(fixDir, (char*)&fileName[4]);
		    strcpy(fileName, fixDir);
		    
		    if(fileName[2] != '/'){	//if we trimmed by accident the only leading / such as 0:filename instead of 0:/filename, restore it so it becomes the latter
				filelen = strlen(fileName)+1;
				char tmpBuf[2];
				tmpBuf[0] = fileName[0];
				tmpBuf[1] = fileName[1];
				
				char fixFile[MAX_TGDSFILENAME_LENGTH+1];       //trim the starting / if it has one
		        memset(fixFile, 0, sizeof(fixFile));
		        strcat(fixFile, tmpBuf);
		        strcat(fixFile, "/");
		        strcat(fixFile, (char*)&fileName[3]);
			}
		}
	}
}

//a variation
static inline void removeLastPath2(char * inPath, char * outPath){
    int len = strlen(inPath);
    if (len < 3){
        outPath[0] = '/';
        outPath[1] = '\0';
        return;
    }
    //remove trailing "/"
    if(inPath[len-1] == '/'){
        inPath[len-1] = '\0';
    }
    int i = 0;
    //remove leading "//"
    if((inPath[i] == '/') && (inPath[i + 1] == '/')){
        i++;
    }
    
    //copy
    while(i < len){
        outPath[i] = inPath[i];
        i++;
    }
    outPath[i] = '\0';
    
    //get and strip last delta
    int wordsToRemove = 0;
    while(outPath[len-wordsToRemove] != '/'){
        wordsToRemove++;
    }
    outPath[len-wordsToRemove] = '\0';
}

static inline bool ShowBrowser(char * Path, char * outBuf){
	scanKeys();
	while((keysPressed() & KEY_START) || (keysPressed() & KEY_A) || (keysPressed() & KEY_B)){
		scanKeys();
		IRQWait(IRQ_VBLANK);
	}
	
	//Create TGDS Dir API context
	struct FileClassList * fileClassListCtx = initFileList();
	cleanFileList(fileClassListCtx);
	
	//Use TGDS Dir API context
	int pressed = 0;
	struct FileClass filStub;
	{
		filStub.type = FT_FILE;
		strcpy(filStub.fd_namefullPath, "");
		filStub.isIterable = true;
		filStub.d_ino = -1;
		filStub.curIndexInsideFileClassList = 0;
		filStub.parentFileClassList = fileClassListCtx;
	}
	char curPath[MAX_TGDSFILENAME_LENGTH+1];
	strcpy(curPath, Path);
	setFileClassObj(0, (struct FileClass *)&filStub, fileClassListCtx);
	
	int j = 1;
	int startFromIndex = 1;
	struct FileClass * fileClassInst = NULL;
	fileClassInst = FAT_FindFirstFile(curPath, fileClassListCtx, startFromIndex);
	while(fileClassInst != NULL){
		//directory?
		if(fileClassInst->type == FT_DIR){
			char tmpBuf[MAX_TGDSFILENAME_LENGTH+1];
			strcpy(tmpBuf, fileClassInst->fd_namefullPath);
			parseDirNameTGDS(tmpBuf);
			strcpy(fileClassInst->fd_namefullPath, tmpBuf);
		}
		//file?
		else if(fileClassInst->type  == FT_FILE){
			char tmpBuf[MAX_TGDSFILENAME_LENGTH+1];
			strcpy(tmpBuf, fileClassInst->fd_namefullPath);
			parsefileNameTGDS(tmpBuf);
			strcpy(fileClassInst->fd_namefullPath, tmpBuf);
		}
		
		//more file/dir objects?
		fileClassInst = FAT_FindNextFile(curPath, fileClassListCtx);
	}
	
	//actual file lister
	clrscr();
	
	j = 1;
	pressed = 0 ;
	int lastVal = 0;
	bool reloadDirA = false;
	bool reloadDirB = false;
	char * newDir = NULL;
	
	#define itemsShown (int)(15)
	int curjoffset = 0;
	int itemRead=1;
	
	while(1){
		int fileClassListSize = getCurrentDirectoryCount(fileClassListCtx) + 1;	//+1 the stub
		int itemsToLoad = (fileClassListSize - curjoffset);
		
		//check if remaining items are enough
		if(itemsToLoad > itemsShown){
			itemsToLoad = itemsShown;
		}
		
		while(itemRead < itemsToLoad ){		
			if(getFileClassFromList(itemRead+curjoffset, fileClassListCtx)->type == FT_DIR){
				printfCoords(0, itemRead, "--- %s%s",getFileClassFromList(itemRead+curjoffset, fileClassListCtx)->fd_namefullPath,"<dir>");
			}
			else{
				printfCoords(0, itemRead, "--- %s",getFileClassFromList(itemRead+curjoffset, fileClassListCtx)->fd_namefullPath);
			}
			itemRead++;
		}
		
		scanKeys();
		pressed = keysPressed();
		if (pressed&KEY_DOWN && (j < (itemsToLoad - 1) ) ){
			j++;
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_VBLANK);
			}
		}
		
		//downwards: means we need to reload new screen
		else if(pressed&KEY_DOWN && (j >= (itemsToLoad - 1) ) && ((fileClassListSize - curjoffset - itemRead) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_DOWN){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_VBLANK);
			}
		}
		
		//LEFT, reload new screen
		else if(pressed&KEY_LEFT && ((curjoffset - itemsToLoad) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset - itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_LEFT){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_VBLANK);
			}
		}
		
		//RIGHT, reload new screen
		else if(pressed&KEY_RIGHT && ((fileClassListSize - curjoffset - itemsToLoad) > 0) ){
			
			//list only the remaining items
			clrscr();
			
			curjoffset = (curjoffset + itemsToLoad - 1);
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_RIGHT){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_VBLANK);
			}
		}
		
		else if (pressed&KEY_UP && (j > 1)) {
			j--;
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_VBLANK);
			}
		}
		
		//upwards: means we need to reload new screen
		else if (pressed&KEY_UP && (j <= 1) && (curjoffset > 0) ) {
			//list only the remaining items
			clrscr();
			
			curjoffset--;
			itemRead = 1;
			j = 1;
			
			scanKeys();
			pressed = keysPressed();
			while(pressed&KEY_UP){
				scanKeys();
				pressed = keysPressed();
				IRQWait(IRQ_VBLANK);
			}
		}
		
		//reload DIR (forward)
		else if( (pressed&KEY_A) && (getFileClassFromList(j+curjoffset, fileClassListCtx)->type == FT_DIR) ){
			struct FileClass * fileClassChosen = getFileClassFromList(j+curjoffset, fileClassListCtx);
			newDir = fileClassChosen->fd_namefullPath;
			reloadDirA = true;
			break;
		}
		
		//file chosen
		else if( (pressed&KEY_A) && (getFileClassFromList(j+curjoffset, fileClassListCtx)->type == FT_FILE) ){
			break;
		}
		
		//reload DIR (backward)
		else if(pressed&KEY_B){
			reloadDirB = true;
			break;
		}
		
		// Show cursor
		printfCoords(0, j, "*");
		if(lastVal != j){
			printfCoords(0, lastVal, " ");	//clean old
		}
		lastVal = j;
	}
	
	//enter a dir
	if(reloadDirA == true){
		//Enter to dir in Directory Iterator CWD
		enterDir((char*)newDir, Path);
		
		//Free TGDS Dir API context
		freeFileList(fileClassListCtx);
		return true;
	}
	
	//leave a dir
	if(reloadDirB == true){
		//Rewind to preceding dir in Directory Iterator CWD
		leaveDir(Path);
		//Free TGDS Dir API context
		freeFileList(fileClassListCtx);
		return true;
	}
	
	strcpy((char*)outBuf, getFileClassFromList(j+curjoffset, fileClassListCtx)->fd_namefullPath);
	clrscr();
	printf("                                   ");
	if(getFileClassFromList(j+curjoffset, fileClassListCtx)->type == FT_DIR){
		//printf("you chose Dir:%s",outBuf);
	}
	else if(getFileClassFromList(j+curjoffset, fileClassListCtx)->type == FT_FILE){
		//printf("you chose Dir:%s",outBuf);
	}
	
	//Free TGDS Dir API context
	freeFileList(fileClassListCtx);
	return false;
}

#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif
