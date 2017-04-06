
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
	// load map of songs
	loadLibrary();
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
	char * pstr;
	int bsent;
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
				
				bsent = sendto(sockUDP, pstr, BUFFSIZE, 0, sendAddr->ai_addr, sendAddr->ai_addrlen);
				cout << "Bytes sent: " << bsent << "\n";
			}
		}
		pstr = cbuff.pop(); //send last pack 
		if (pstr)
			bsent = sendto(sockUDP, pstr, lastpsz, 0, sendAddr->ai_addr, sendAddr->ai_addrlen);
	}
}

int Server::runServer(const char * ipaddr)
{
	//tcp socket , make , bind, listen , accpet .. 
	// send lib
	// sned client list 
	// listen to commands
	// if the client wants to stream a library, get index of lib  
	int lib = 0;
	// make a new thread to start stream
	startStream(lib);
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
