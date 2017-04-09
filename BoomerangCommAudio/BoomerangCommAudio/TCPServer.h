/*------------------------------------------------------------------------------
-- SOURCE FILE: TCPServer.h - The TCP server class
--
-- PROGRAM: BoomerangCommAudio.exe
--
-- FUNCTIONS:
--		void AcceptFunc();
--		void RunServer(SOCKET& serverSock);
--		bool SendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo);
--		void WorkThread();
--
-- DATE: Apr. 09, 2017
--
-- REVISIONS:
-- Version 1.0 - [luxes] - Apr. 09, 2017
-- DESIGNER: Jamie Lee
--
-- PROGRAMMER: Jamie Lee
--
-- NOTES:
-- The class to run a tcp server.
------------------------------------------------------------------------------*/

#pragma once

#include <Winsock2.h>
#include <map>

#include "Common.h"

#define MAX_NUM_CLIENT 5

struct ClientInformation
{
	char username[PACKET_STR_MAX];
	char ip[IP_LENGTH];
};

class TCPServer
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
	~TCPServer();

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