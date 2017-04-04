#pragma once

#include "CBuff.h"
#include "Packetizer.h"
#include "SocketWrappers.h"


#define BUFFSIZE 1024
#define MCAST_IP "299.99.99.09"
#define TEMP_IP "192.168.0.36"

//The following function was set up for testing 
int runServer(const char * ipaddr = TEMP_IP, int numPacks = 100, int timeinterval = 500);


/***************************
 Server Class Construction
 __      __._____________ 
/  \    /  \   \______   \
\   \/\/   /   ||     ___/
 \        /|   ||    |    
  \__/\  / |___||____|    
       \/              
Your patience is appreciated  
***************************/ 
class Server 
{
public:
	Server();
	int runServer(const char * ipaddr);

private:
	SOCKET sockUDP;
	HANDLE hTimer;
	LARGE_INTEGER dueTime;
	SoundFilePacketizer packer;
	long totalPacksForSong;
	CBuff cbuff;
};