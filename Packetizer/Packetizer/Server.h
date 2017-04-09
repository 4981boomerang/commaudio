/*------------------------------------------------------------------------------
-- SOURCE FILE: Server.h - The server header
--
-- PROGRAM: COMP4985 A4
--
-- FUNCTIONS:
-- 		Server() 
-- 		~Server()
-- 		void stopStream()
-- 		void startStream()
-- 		int runServer(const char * ipaddr)
-- 		inline bool waitForTimer()
-- 		void loadLibrary(const char * libpath = LIB_DIR_PATH)
-- 		void streamSendLoop()
-- 		void streamPackLoop()
-- 		inline std::thread streamSend()
-- 		inline std::thread streamPack()
--
-- DATE: APR. 09, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/09 - Created Comments 

-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- Server for audio streaming and sending audio files
-- streaming is multicast streaming 
-- sending audio files is via direct TCP connection
------------------------------------------------------------------------------*/
#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <mutex>
#include <map>
#include <queue>
#include <thread>

#include "CBuff.h"
#include "Packetizer.h"
#include "WinTimer.h"
#include "SocketWrappers.h"

#define BUFFSIZE 1024
#define PORTNO 5001
#define MCAST_IP "295.99.99.09"
#define TEMP_IP "192.168.0.36"
#define LIB_DIR_PATH "C:\\Users\\Eva\\Documents\\CST\\Semester4\\comp4985\\assignments\\a4\\Radiohead\\"

class Server 
{
public:
	Server();
	~Server();
	void stopStream();
	void startStream();
	int runServer(const char * ipaddr);
	inline bool waitForTimer();
	void loadLibrary(const char * libpath = LIB_DIR_PATH);
private:
	
	void streamSendLoop();
	void streamPackLoop();

	/*--------------------------------------------------------------------------
	-- FUNCTION: streamSend
	--
	-- DATE: APR. 09, 2017
	--
	-- REVISIONS: 
	-- Version 1.0 - [EY] - 2016/APR/09 
	--
	-- DESIGNER: Someone from Stack Overflow
	--
	-- PROGRAMMER: Eva Yu
	--
	-- INTERFACE: inline  streamSend ()
	--  
	--
	-- RETURNS: 
	-- a thread to the function called
	--
	-- NOTES:
	-- inline function that starts a thread by using a lambda 
	-- used to get around initiating class functions 
	-- for threads
	-- this function is for sending during streaming
	--------------------------------------------------------------------------*/
	inline std::thread streamSend() {
		return std::thread([this] { this->streamSendLoop(); });
	};

	/*--------------------------------------------------------------------------
	-- FUNCTION: streamPack
	--
	-- DATE: APR. 09, 2017
	--
	-- REVISIONS: 
	-- Version 1.0 - [EY] - 2016/APR/09 
	--
	-- DESIGNER: Someone from Stack Overflow
	--
	-- PROGRAMMER: Eva Yu
	--
	-- INTERFACE: inline  streamPack ()
	--  
	--
	-- RETURNS: 
	-- a thread to the function called
	--
	-- NOTES:
	-- inline function that starts a thread by using a lambda 
	-- used to get around initiating class functions 
	-- for threads
	-- this function is for packing during streaming
	--------------------------------------------------------------------------*/
	
	inline std::thread streamPack() {
		return std::thread([this] { this->streamPackLoop(); });
	};

	// SOCKET sockTCP;	
	SOCKET sockUDP;
	std::map <int,std::string> playlist; // the library
	WinTimer timer;
	std::thread sendThread;
	std::thread packThread;
	SoundFilePacketizer packer;
	CBuff cbuff;
	bool isStreaming;
};