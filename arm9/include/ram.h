//============================================================================//
//                                                                            //
//  Copyright 2007 Rick "Lick" Wong                                           //
//                                                                            //
//  This library is licensed as described in the included readme.             //
//                                                                            //
//============================================================================//


#ifndef __RAM
#define __RAM

#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif


//typedef enum { DETECT_RAM=0, SC_RAM, M3_RAM, OPERA_RAM, G6_RAM, EZ_RAM } RAM_TYPE;

//  Call this before the others
vu16*  ram_init ();

//  Returns the type of the RAM device
u32   ram_type ();

//  Returns the type of the RAM device in a string
const char*   ram_type_string ();

//  Returns the total amount of RAM in bytes
u32   ram_size ();


//  Unlocks the RAM and returns a pointer to the begin
vu16* ram_unlock ();

//  Locks the RAM
void  ram_lock ();


#ifdef __cplusplus
}
#endif
#endif
