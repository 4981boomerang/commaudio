
#include <iostream>
#include <string.h>
#include <thread>
#include <iterator>

#include "Server.h"

using namespace std;

Server::Server()
	:timer()
{
	initializeWSA();
	// load map of songs
	sockUDP = makeWSASocket(SOCK_DGRAM, 0);

	loadLibrary();
}

Server::~Server()
{
	if (isStreaming);
		stopStream();

	closeWSA();
}

void Server::startStream()
{

	// get file name library
	// make thread for streaming
	isStreaming = true;
	packThread = streamPack();
	sendThread = streamSend();
	//thread sndthread(streamSendLoop, this);
	// Make a thread for sending 
	//thread sndthread(streamSendLoop, this);
	return;
}

void Server::loadLibrary()
{
	static int sid = 0;
	playlist[sid++] = "C:\\Users\\Eva\\Documents\\CST\\Semester4\\comp4985\\assignments\\a4\\Radiohead\\01Just.mp3";
	playlist[sid++] = "C:\\Users\\Eva\\Documents\\CST\\Semester4\\comp4985\\assignments\\a4\\Radiohead\\02MoaningLisaSmile.mp3";
	playlist[sid++] = "C:\\Users\\Eva\\Documents\\CST\\Semester4\\comp4985\\assignments\\a4\\Radiohead\\03KarmaPolice.mp3";
	playlist[sid++] = "C:\\Users\\Eva\\Documents\\CST\\Semester4\\comp4985\\assignments\\a4\\Radiohead\\04Creep.mp3";
}

void Server::stopStream()
{
	isStreaming = false;
	//join thread stream Send Loop
	sendThread.join();
	packThread.join();
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
				if (!isStreaming) break;
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
	const long long initialLoadInterval = -20000LL; // 2 MS
	const long long initialLoadSize = 21;
	const long long regularLoadInterval = -100000LL; // 10 MS
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, TEMP_IP/*MCAST_IP*/, &(addr.sin_addr.s_addr));
	addr.sin_port = htons(PORTNO);
	
	while (isStreaming)
	{
		//get info of next song
		long ttl = packer.getTotalPackets();
		long lastpsz = packer.getLastPackSize();
		timer.cancelTimer();
		timer.setTimer(initialLoadInterval);
		timer.resetTimer();
		//end the song
		for (long i = 0; i < ttl; ++i)
		{
			if (!isStreaming) break;
			if(i == initialLoadSize)
			{
				timer.cancelTimer();
				timer.setTimer(regularLoadInterval);
				timer.resetTimer();
			}

			if ( waitForTimer() )
			{
				pstr = cbuff.pop(); // get next pack 
				if (!pstr)
					continue;
				
				bsent = sendto(sockUDP, pstr, BUFFSIZE, 0, (struct sockaddr *)&addr, sizeof(addr));
				cout << "Bytes sent: " << bsent << "\n";
			}
		}
		if (!isStreaming) break;
		pstr = cbuff.pop(); //send last pack 
		if (pstr)
			bsent = sendto(sockUDP, pstr, lastpsz, 0, (struct sockaddr *)&addr, sizeof(addr));
			cout << "Bytes sent: " << bsent << "\n";
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
