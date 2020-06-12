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
#include "spiTGDS.h"
#include "typedefsTGDS.h"
#include "dsregs.h"

uint8 RWSPICNT(uint8 val){
	REG_SPI_DATA = (val & BITMASK_SPI_DATA);
	SPIWAITCNT();
	return (REG_SPI_DATA);
}

//Acknowledges a SPI command
void SPICSHIGH(){
	REG_SPI_CR = 0;
}

void SPIWAITCNT(){
	while(REG_SPI_CR & BIT_SPICNT_BUSY);
}
