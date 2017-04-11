
#include <iostream>
#include <string.h>
#include <thread>
#include <iterator>

#include "Server.h"

using namespace std;

/*--------------------------------------------------------------------------
-- FUNCTION: Server
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Comment Added 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  Server ()
--
-- NOTES:
-- Ctor
--------------------------------------------------------------------------*/
Server::Server()
	:isStreaming(false)
{
	initializeWSA();
	// load map of songs
	sockUDP = makeWSASocket(SOCK_DGRAM, 0);
}

/*--------------------------------------------------------------------------
-- FUNCTION: Server
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Comment Added 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  Server ()
--
-- NOTES:
-- Dtor
--------------------------------------------------------------------------*/
Server::~Server()
{
	if (isStreaming)
		stopStream();

	closeWSA();
}

/*--------------------------------------------------------------------------
-- FUNCTION: startStream
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Comment added 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void startStream ()
--  desc
--
-- RETURNS: 
-- RETURN
--
-- NOTES:
-- Starts the UDP Streaming of songs. 
-- creates a thread for packetizatio and a thread for streaming 
-- **** make sure the Library is loaded when calling start stream !
--------------------------------------------------------------------------*/
void Server::startStream()
{

	// get file name library
	isStreaming = true;
	// make threads for streaming 1 for pack , 1 for send
	packThread = streamPack();
	sendThread = streamSend();

	return;
}

/*--------------------------------------------------------------------------
-- FUNCTION: loadLibrary
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Commented the code 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  loadLibrary (const char * libpath)
-- const char * libpath : The path the the directory of the library
--							The deafult for the path is in server.h 
--
--
-- NOTES:
-- This function loads  all the mp3 files found in the folder
-- * Future versions may also extend to .wav , .flac , etc . 
--------------------------------------------------------------------------*/
void Server::loadLibrary(const char * libpath)
{
	// look for mp3 files only
	static const char * rgx = "*.mp3";

	// if library has already been loaded, 
	if (playlist.size() != 0)
	{
		if (isStreaming) //you must stop streaming first!
		{
			cerr << "Streaming must stop before loading library."
				<< endl;
			return;
		}
		else {
			playlist.clear();
		}
	}
	
	int sid = 0; // song id assiocated with map 
	string path = "";
	(path = libpath).append(rgx);
	WIN32_FIND_DATA winfd;
	HANDLE hFind = FindFirstFile(path.c_str(), &winfd);
	if (hFind != INVALID_HANDLE_VALUE) { 
		
		do {
			// if it is a file and not a directory
			if (!(winfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				// add file path and name of file to map
				(playlist[sid++] = libpath).append(winfd.cFileName);
			}
		} while (FindNextFile(hFind, &winfd));
		FindClose(hFind);
	}
}

/*--------------------------------------------------------------------------
-- FUNCTION: stopStream
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Commented  
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void stopStream ()
--  
--
-- RETURNS: 
--
-- NOTES:
-- called when the streaming wants to be aborted entirely ( Note, this is not a pause )
--------------------------------------------------------------------------*/
void Server::stopStream()
{
	isStreaming = false;
	//join thread stream Send Loop
	sendThread.join();
	packThread.join();
	//join thread Stream Pack Loop
}

/*--------------------------------------------------------------------------
-- FUNCTION: streamPackLoop
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Comment added 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void streamPackLoop ()
--  
--
-- NOTES:
-- this is a the thread loop for 
-- thread will exit when somone stops streaming 
-- Continuously loop through the map of songs ( loop back play )
-- and packetize each song 
-- Once the cBuff has the last packet of the song, the next song will 
-- be packetized. 
--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------
-- FUNCTION: streamSendLoop
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Comment 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  streamSendLoop ()
--
-- NOTES:
-- This is the thread loop for the streaming.
-- the loop will exit when the flag exits  
-- the initial 21K of a song is sent every 2 milliseconds
-- after that, the data slows down at sending rate that matches the speed
-- of streaming 
--------------------------------------------------------------------------*/
void Server::streamSendLoop()
{
	sockaddr_in addr;
	char * pstr;
	int bsent;
	const long long initialLoadInterval = -20000LL; // 2 MS
	const long long initialLoadSize = 21;
	const long long regularLoadInterval = -1000000LL; // 100 MS
	
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, MCAST_IP, &(addr.sin_addr.s_addr));
	addr.sin_port = htons(PORTNO);
	
	while (isStreaming)
	{
		//get info of next song
		long ttl = packer.getTotalPackets();
		long lastpsz = packer.getLastPackSize();
		timer.cancelTimer();
		timer.setTimer(initialLoadInterval);
		
		//end the song
		for (long i = 0; i < ttl; ++i)
		{
			if (!isStreaming) break;
			// if it is no longer the intial part of the song
			if(i == initialLoadSize)
			{
				// reset due time to regulkar mp3 rates
				timer.cancelTimer();
				timer.setTimer(regularLoadInterval);
			}

			timer.resetTimer();
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

/*--------------------------------------------------------------------------
-- FUNCTION: runServer
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Add comments 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: int runServer (const char * ipaddr)
-- const char * ipaddr 
--
-- RETURNS: 
-- int representing state
--
-- NOTES:
-- this is called right after the very initial constructor
-- meant for the TCP thread to start 
--
-- THIS IS AWAITING THE ADDITIONS FROM TCP SIDE
--------------------------------------------------------------------------*/
int Server::runServer(const char * ipaddr)
{
	//tcp socket , make , bind, listen , accpet .. 
	// send lib
	// send client list 
	// listen to commands
	// if the client wants to stream a library, get index of lib  
	// make a new thread to start stream
	startStream();
	return 0;
}

/*--------------------------------------------------------------------------
-- FUNCTION: waitForTimer
--
-- DATE: APR. 09, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Comment  
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: inline bool waitForTimer ()
--  desc
--
-- RETURNS: 
-- boolean -- whether the timer object tiggered the event 
--
-- NOTES:
-- a wait for event function wrapper 
-- that specifically waits for a timer object
--------------------------------------------------------------------------*/
inline bool Server::waitForTimer()
{
	if (WaitForSingleObject(timer.getTimer(), INFINITE) != WAIT_OBJECT_0)
	{
		printf("WaitForSingleObject failed (%d)\n", GetLastError());
		return false;
	}
	return true;
}
