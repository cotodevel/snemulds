/*
Copyright (C) 2015-2017  Coto

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/


//nifi-udp netplay code. Todo
/*
#ifdef __cplusplus
extern "C"{
#endif

//0 not ready, 1 act as a host and waiting, 2 act as a guest and waiting, 3 connecting, 4 connected, 5 host ready, 6 guest ready
extern int nifi_stat;
extern int nifi_cmd;
extern int nifi_keys;		//holds the keys for players.
extern int new_nkeys;		//holds the next key for refreshing.
extern int old_nkeys;		//if no new_keys coming, use the old_keys
extern int plykeys1;		//player1
extern int plykeys2;		//player2
extern int new_framecount;	//used by the guest for syncing.
extern int guest_framecount;	//used by the guest for syncing.
extern int old_guest_framecount;	//used by the guest for syncing.
//extern int framecount;

extern int host_framecount;
extern int guest_framecount;

extern char key_maps[16];			//for guest to record the key_buf state.
extern int key_buf[16];			//for guest to record the keys...

extern int frame_stuck;		//ont player should not be stuck when the other failed to comm...

extern int host_vcount;		//host generated REG_VCOUNT
extern int guest_vcount;		//guest generated REG_VCOUNT

extern int topvalue(int a,int b);
extern int bottomvalue(int a,int b);
extern int getintdiff(int a,int b);
extern int getnifivcount();

extern bool waitfornesmulti_sync();

extern int nifi_keys_sync;	//(guestnifikeys & hostnifikeys)

extern bool nifi_frame_served;

extern void sendcmd(u8 * databuf_src);	//framesize is calculated inside
extern void getcmd(u8 * databuf_src);	//framesize is calculated inside (crc over udp requires framesize previously to here calculated anyway)


#ifdef __cplusplus
}
#endif
*/