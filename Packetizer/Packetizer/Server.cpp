
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <iostream>
#include <string.h>
#include <thread>

#include "Server.h"
#include "CBuff.h"
#include "Packetizer.h"
#include "SocketWrappers.h"

using namespace std;

Server::Server()
{
	initializeWSA();
	// load map of songs
	sockUDP = makeWSASocket(SOCK_DGRAM, 0);

	loadLibrary();
}

Server::~Server()
{
	// join the threads

	closeWSA();
}

void Server::startStream()
{

	// get file name library
	//make thread for streaming
	isStreaming = true;
	//thread sndthread(streamSendLoop, this);
	// Make a thread for sending 
	//thread sndthread(streamSendLoop, this);
	return;
}

void Server::loadLibrary()
{
	static int sid = 0;
	playlist[sid++] = "test";
}

void Server::stopStream()
{
	isStreaming = false;
	//join thread stream Send Loop
	//join thread Stream Pack Loop
}

void Server::streamPackLoop()
{
	while (isStreaming)
	{
		//continuously loop through the map of songs
		for (std::map<int, std::string>::iterator it = playlist.begin(); it != playlist.end(); ++it)
		{
			packer.makePacketsFromFile(it->second.c_str());
			long ttl = packer.getTotalPackets();
			for (long i = 0; i < ttl; ++i)
			{
				cbuff.push_back(packer.getNextPacket());
			}
		}
	}
}

void Server::streamSendLoop()
{
	sockaddr_in addr;
	char * pstr;
	int bsent;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(MCAST_IP);
	addr.sin_port = htons(PORT);
	while (isStreaming)
	{
		//get info of next song
		long ttl = packer.getTotalPackets();
		long lastpsz = packer.getLastPackSize();

		//end the song
		for (long i = 0; i < ttl; ++i)
		{
			timer.resetTimer();
			if (waitForTimer())
			{
				pstr = cbuff.pop(); // get next pack 
				if (!pstr)
					continue;
				
				bsent = sendto(sockUDP, pstr, BUFFSIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
				cout << "Bytes sent: " << bsent << "\n";
			}
		}
		pstr = cbuff.pop(); //send last pack 
		if (pstr)
			bsent = sendto(sockUDP, pstr, lastpsz, 0, (struct sockaddr *)&addr, sizeof(addr));
	}
}

int Server::runServer(const char * ipaddr)
{
	//tcp socket , make , bind, listen , accpet .. 
	// send lib
	// send client list 
	// listen to commands
	// if the client wants to stream a library, get index of lib  
	int lib = 0;
	// make a new thread to start stream
	startStream();
	return 0;
}

inline bool Server::waitForTimer()
{
	if (WaitForSingleObject(timer.getTimer(), INFINITE) != WAIT_OBJECT_0)
	{
		printf("WaitForSingleObject failed (%d)\n", GetLastError());
		return false;
	}
	return true;
}
