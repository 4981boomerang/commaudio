#pragma once

#include <winsock2.h>
#include <mutex>
#include <map>
#include <queue>
#include <thread>

#include "CBuff.h"
#include "Packetizer.h"
#include "WinTimer.h"
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
	void stopStream();
	void startStream();
	int runServer(const char * ipaddr);
	inline bool waitForTimer();
private:
	void loadLibrary();
	void streamSendLoop();
	void streamPackLoop();
	std::map <int,std::string> playlist; // the library
	
	// Map of different types of libraries 
	// SOCKET sockTCP;	
	SOCKET sockUDP;
	WinTimer timer;
	std::thread streamSend;
	std::thread streamPack;
	SoundFilePacketizer packer;
	CBuff cbuff;
	addrinfo * sendAddr;
	std::mutex imtx;
	bool isStreaming;
};