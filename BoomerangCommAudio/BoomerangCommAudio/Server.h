/*------------------------------------------------------------------------------
-- SOURCE FILE: Server.h - The server class
--
-- PROGRAM: BoomerangCommAudio.exe
--
-- FUNCTIONS:
--		void AcceptFunc()
--		void RunServer(SOCKET& serverSock)
--		bool SendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo)
--		void WorkThread()
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
-- DATE: Apr. 09, 2017
--
-- REVISIONS:
-- Version 1.0 - [luxes] - Apr. 09, 2017
-- Version 2.0 - [EY] - Apr. 10, 2017 - merged the UDP Stream Code
--
-- DESIGNER: Jamie Lee
--
-- PROGRAMMER: Eva Yu & Jamie Lee
--
-- DESIGNER: Eva Yu
--
-- NOTES:
-- The class to run a tcp server.
------------------------------------------------------------------------------*/

#pragma once

#include <Winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <map>
#include <string>

#include "Common.h"
#include "CBuff.h"
#include "Packetizer.h"
#include "WinTimer.h"
#include "SocketWrappers.h"

#define MAX_NUM_CLIENT 5
#define BUFFSIZE 1024
#define PORTNO 5001
#define MCAST_IP "235.0.0.1"
#define LIB_DIR_PATH "C:\\Users\\Eva\\Documents\\CST\\Semester4\\comp4985\\assignments\\a4\\Radiohead\\"

struct ClientInformation
{
	char username[PACKET_STR_MAX];
	char ip[IP_LENGTH];
};

class Server
{
private:
	LPSOCKET_INFORMATION SocketInfo;
	SOCKET tcp_listen;
	std::map<SOCKET, ClientInformation> mapClient;
	WSAEVENT AcceptEvent;
	DWORD EventTotal = 0;
	WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
	LPSOCKET_INFORMATION SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
	CRITICAL_SECTION CriticalSection;

	SOCKET sockUDP;
	std::map <int, std::string> playlist; // the library
	WinTimer timer;
	std::thread sendThread;
	std::thread packThread;
	SoundFilePacketizer packer;
	CBuff cbuff;
	bool isStreaming;

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

	/*------------------------------------------------------------------------------
	-- FUNCTION: AcceptFunc
	--
	-- DATE: Apr. 09, 2017
	--
	-- REVISIONS:
	-- 1.0  - [luxes] - Description
	--
	-- DESIGNER: Jamie Lee
	--
	-- PROGRAMMER: Jamie Lee
	--
	-- INTERFACE: void AcceptFunc();
	--
	-- RETURNS:
	-- void
	--
	-- NOTES:
	-- Accepts a connection and keep it in the map.
	--------------------------------------------------------------------------*/
	void AcceptFunc();

	/*------------------------------------------------------------------------------
	-- FUNCTION: SendInitialInfo
	--
	-- DATE: Apr. 09, 2017
	--
	-- REVISIONS:
	-- 1.0  - [luxes] - Description
	--
	-- DESIGNER: Jamie Lee
	--
	-- PROGRAMMER: Jamie Lee
	--
	-- INTERFACE: void SendInitialInfo(SOCKET socket, LPSOCKET_INFORMATION SocketInfo);
	-- socket: A socket to send information.
	-- SocketInfo: The information of the socket.
	--
	-- RETURNS:
	-- void
	--
	-- NOTES:
	-- Sends initial messages such as the list of clients and songs.
	--------------------------------------------------------------------------*/
	void SendInitialInfo(SOCKET socket, LPSOCKET_INFORMATION SocketInfo);

public:

	Server();
	~Server();
	void stopStream();
	void startStream();
	inline bool waitForTimer();
	void loadLibrary(const char * libpath = LIB_DIR_PATH);

	/*------------------------------------------------------------------------------
	-- FUNCTION: RunServer
	--
	-- DATE: Apr. 09, 2017
	--
	-- REVISIONS:
	-- 1.0  - [luxes] - Description
	--
	-- DESIGNER: Jamie Lee
	--
	-- PROGRAMMER: Jamie Lee
	--
	-- INTERFACE: void RunServer(SOCKET& serverSock);
	-- serverSock: A socket for a TCP server.
	--
	-- RETURNS:
	-- void
	--
	-- NOTES:
	-- Initializes a TCP server, including creating and binding a socket.
	-- Starts two threads for an accept and receive.
	--------------------------------------------------------------------------*/
	void RunServer(SOCKET& serverSock);

	/*------------------------------------------------------------------------------
	-- FUNCTION: SendTCP
	--
	-- DATE: Apr. 09, 2017
	--
	-- REVISIONS:
	-- 1.0  - [luxes] - Description
	--
	-- DESIGNER: Jamie Lee
	--
	-- PROGRAMMER: Jamie Lee
	--
	-- INTERFACE: bool SendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo);
	-- clientSock: A socket to send a message.
	-- SocketInfo: The information of the socket.
	--
	-- RETURNS:
	-- bool: Returns true when successfully sent. Otherwise, returns false.
	--
	-- NOTES:
	-- Sending a message in DataBuf in SocketInfo via TCP.
	--------------------------------------------------------------------------*/
	bool SendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo);

	/*------------------------------------------------------------------------------
	-- FUNCTION: WorkThread
	--
	-- DATE: Apr. 09, 2017
	--
	-- REVISIONS:
	-- 1.0  - [luxes] - Description
	--
	-- DESIGNER: Jamie Lee
	--
	-- PROGRAMMER: Jamie Lee
	--
	-- INTERFACE: void WorkThread();
	--
	-- RETURNS:
	-- void
	--
	-- NOTES:
	-- Wait for multiple events via TCP. When a message arrived,
	-- it complets the message in WorkerRoutine.
	--------------------------------------------------------------------------*/
	void WorkThread();
};

/*------------------------------------------------------------------------------
-- FUNCTION: WorkerRoutine
--
-- DATE: Apr. 09, 2017
--
-- REVISIONS:
-- 1.0  - [luxes] - Description
--
-- DESIGNER: Jamie Lee
--
-- PROGRAMMER: Jamie Lee
--
-- INTERFACE: void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
--
-- RETURNS:
-- void
--
-- NOTES:
-- Complets the event that is from TCP.
--------------------------------------------------------------------------*/
void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);