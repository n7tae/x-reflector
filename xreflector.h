#pragma once
/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Original software by Scot Lawson, KI4LKF
// Copyright (C) 2018 by Thomas A. Early, N7TAE

//#include <stdio.h>
//#include <fcntl.h>
//#include <string.h>
//#include <ctype.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <signal.h>
//#include <errno.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <netinet/in.h>
//#include <netdb.h>
//#include <time.h>
//#include "GitVersion.h"

#include <regex.h>

//#include <unistd.h>
//#include <sys/socket.h>
//#include <sys/ioctl.h>
//#include <arpa/inet.h>

/* Required for Binary search trees using C++ STL */
#include <string>
#include <map>
#include <set>
//#include <utility>

#include <pthread.h>

#define VERSION "5.00"
#define OWNER_SIZE 8
#define IP_SIZE 15
#define MAXHOSTNAMELEN 64
#define CALL_SIZE 8
#define LH_MAX_SIZE 39
#define MAX_RCD_USERS 30
#define MAX_RCD_DATA 750
#define READBUFFER_SIZE 1024

/*
   Timeout is 30 seconds.
   If after 30 seconds, we have not received the KEEPALIVE,
   we drop that station
*/
struct inbound {
	char call[CALL_SIZE + 1];	// the callsign of the remote
	bool isMute;				// if true, packets from this call are dropped
	time_t connect_time;
	struct sockaddr_in sin;		// IP and port of remote
	short countdown;			// if countdown expires, the connection is terminated
	char mod;					// A B C D E This user talked on this module
	bool is_ref;
	char serial[9];				// the serial number of the dongle
	char links[5];	// The index is the local module: 0=A, 1=B, 2=C, 3=D, 4=E
					// The value is the remote module
					// Example:  links[1] = 'C'
					// That means that our local module B is linked to remote module C
					// The remote system is identified by inbound->call
};


struct a_user {
	char call[CALL_SIZE + 1];	// callsign of the connected repeater
	bool isMute;				// if true, packets from this call are dropped
	bool is_xrf;				// is this another XRF reflector
	time_t connect_time;

	/* The first index identifies the local reflector module
	index 0 identifies reflector module A
	index 1 identifies reflector module B
	etc...

	The second index identifies the remote repeater band.
	from 0 to 3
	which is from A...D

	Example:
	rpt_mods[1][2] = 'C'
	rpt_mods[1][3] = 'D'

	Explanation:
	Reflector module B is linked to repeater module C
	Reflector module B is linked to repeater module D
	This means that the remote repeater
	has linked both repeater bands C and D to our reflector module B
	*/
	char rpt_mods[5][4];

	time_t link_time[5][4];	// time link was established
	struct sockaddr_in sin;	// IP address and UDP port of connected station' For easy access to the connected station.
	short countdown;		// if countdown expires, the connection is terminated
	char mod;				// This user talked on this module
};

struct rcd
{
	bool locked;
	time_t ts;
	struct sockaddr_in sin;
	short int recvlen;
	unsigned short idx; 					// index into data
	unsigned char data[MAX_RCD_DATA][58];	// 10 seconds
};

class CXReflector {
public:
	CXReflector() {}
	~CXReflector() {}
	bool Initialize(int argc, char **argv);
	void Run();
	void Stop();
	regex_t preg;
private:
	short TIMEOUT = 60;
	short pwunlock = 0;
	time_t unlocktime;
	char STATUS_FILE[FILENAME_MAX + 1];

	/* configuration data */
	/* Put that in a structure, later */
	char OWNER[OWNER_SIZE + 1];
	char ADMIN[CALL_SIZE + 1];
	char LISTEN_IP[IP_SIZE + 1];
	int LISTEN_PORT = 30001;
	int COMMAND_PORT = 30010;

	/* max number of XRF repeaters */
	unsigned int MAX_USERS = 10;

	/* max number of dvap/dvtool dongles */
	unsigned int MAX_OTHER_USERS=10;

	bool QSO_DETAILS = false;
	char USERS[FILENAME_MAX + 1];
	char BLOCKS[FILENAME_MAX + 1];

	// Input from XRF repeaters
	int srv_sock = -1;

	// Input from admin commands
	int cmd_sock = -1;

	// Input from dvap, dvtool dongles
	int ref_sock = -1;

	/* ACK back to dvap, dvtool users */
	unsigned char REF_ACK[3] = { 3, 96, 0 };

	std::map<std::string, std::string> dt_lh_list;	// for replying with DASHBOARD information

	// inbound dongles(dvap, dvtool, ...) on port 20001
	// the Key in this inbound_list map is the unique IP-port address of the remote, example:  x.x.x.x-20001
	std::map<std::string, inbound *> inbound_list;

	// Just before we send data to dvtool/dvap users, we save the header here, for re-transmit later
	struct {
		uint32_t s_addr;
		unsigned char hdr[58];
	} temp_r[5];

	// The BST/map of connected users
	// The KEY is the unique IP address string of ip[IP_SIZE + 1]
	// The data is a pointer to struct a_user
	std::map<std::string, struct a_user *> a_user_list;

	// Just before we send the data to XRF repeaters
	// we save the header here for re-transmit later
	struct temp_tag {
		uint32_t s_addr;
		unsigned char hdr[56];
		unsigned char old_sid[2];
	} temp_x[5];

	std::map<std::string, std::string> call_ip_map;	// the map of which reflectors we can link to

	// The BST/set of blocked callsigns.
	// The KEY is the unique blocked callsign.
	// There is no data in the set.
	// Blocked callsigns added by the administrator of the reflector.
	std::set<std::string> blocks;

	fd_set fdset;	// socket descriptor set
	struct timeval tv;
	time_t tNow;
	time_t HBinterStart;		// timing for XRF users
	time_t inboundStart;		// timing for dvtool/dvap users

	// Variables used by more than one function
	unsigned char readBuffer[READBUFFER_SIZE];
	struct sockaddr_in fromUser;
	struct sockaddr_in fromCmd;

	/* dvap, dvtool input buffer */
	unsigned char refbuf[READBUFFER_SIZE];
	struct sockaddr_in fromInbound;
	FILE *statusfp = NULL;	// status file

	/*** rcd data ***/
	pthread_t playback_thread;
	pthread_attr_t attr;

	std::map<std::string, struct rcd *> rcd_list;	// key is streamid
	struct rcd *an_rcd;
	std::map<std::string, struct rcd *>::iterator rcd_pos;
	std::pair<std::map<std::string, struct rcd *>::iterator, bool> rcd_insert_pair;
	char an_rcd_streamid[32];
	time_t check_rcd_time = 0;

	bool keep_running = true;
	u_int16_t streamid_raw = 0;

	// The reflector uses these functions only
	void check_heartbeat();
	void print_users();
	void print_version();
	void mute_users(bool mute);
	bool mute_call(char *call, bool mute);
	void print_blocks();
	void print_links_file();
	void print_links_screen();
	bool get_ip(char *call, char *ip);
	void handle_cmd(char *buf);
	void runit();
	int  read_config(char *);
	int  srv_open();
	int  cmd_open();
	void sigCatch(int signum);
	int  open_users(char *filename);
	int  open_blocks(char *filename);
	bool resolve_rmt(char *name, int type, struct sockaddr_in *addr);
	void playback(void *arg);

	// dvap, dongles
	void send_heartbeat();
	int  ref_open();
	int  link_to_ref(char *call);
	int  link_to_xrf(char local_mod, char *ref, char remote_mod, char *IP);
};
