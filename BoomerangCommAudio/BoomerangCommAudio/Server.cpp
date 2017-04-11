#include "stdafx.h"
#include "Server.h"
#include "SocketWrappers.h"

//#include <WinBase.h>
#include <string>
#include <thread>
#include <iostream>
#include <string.h>
#include <iterator>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

//using namespace std;
using std::cerr;
using std::cout;
using std::endl;
using std::string;

// initialize static memebers.
WSAEVENT Server::EventArray[WSA_MAXIMUM_WAIT_EVENTS];
LPSOCKET_INFORMATION Server::SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
CRITICAL_SECTION Server::CriticalSection;
DWORD Server::EventTotal = 0;
std::map<SOCKET, ClientInformation> Server::mapClient;

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
	GlobalFree(SocketInfo);
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
			if (i == initialLoadSize)
			{
				// reset due time to regulkar mp3 rates
				timer.cancelTimer();
				timer.setTimer(regularLoadInterval);
			}

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
		if (!isStreaming) break;
		pstr = cbuff.pop(); //send last pack 
		if (pstr)
			bsent = sendto(sockUDP, pstr, lastpsz, 0, (struct sockaddr *)&addr, sizeof(addr));
		cout << "Bytes sent: " << bsent << "\n";
	}
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

/*****************************************************************************
								Jamies portion
*****************************************************************************/
void Server::RunServer(SOCKET& serverSock)
{
	char temp[STR_SIZE];

	InitializeCriticalSection(&CriticalSection);

	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		sprintf_s(temp, STR_SIZE, "GlobalAlloc() failed with error %d", GetLastError());
		Display(temp);
		return;
	}

	SocketInfo->BytesRECV = 0;
	SocketInfo->BytesSEND = 0;
	SocketInfo->PacketsRECV = 0;
	SocketInfo->DataBuf.len = BUF_SIZE;

	if (!initializeWSA())
		return;

	// create a socket.
	if ((tcp_listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return;

	struct sockaddr_in server;
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(g_port);
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	// Bind an address to the socket
	if ( bind(tcp_listen, (struct sockaddr *)&server, sizeof(server)) == -1)
		return;

	if (listen(tcp_listen, MAX_NUM_CLIENT) == SOCKET_ERROR)
	{
		sprintf_s(temp, STR_SIZE, "listen() failed with error %d", WSAGetLastError());
		Display(temp);
		return;
	}

	/*if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		sprintf_s(temp, STR_SIZE, "WSACreateEvent() failed with error %d", WSAGetLastError());
		Display(temp);
		return;
	}
	AcceptEvent = EventArray[0];*/

	sprintf_s(temp, STR_SIZE, "Listen TCP port %d", g_port);
	Display(temp);

	std::thread threadAccept(&Server::AcceptFunc, this);
	threadAccept.detach();

	std::thread workThread(&Server::WorkThread, this);
	workThread.detach();
}

void Server::AcceptFunc()
{
	int client_len;
	struct	sockaddr_in client;
	char temp[STR_SIZE];

	if ((AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return;
	}

	EventTotal = 1;

	while (true)
	{
		client_len = sizeof(client);
		//SOCKET acceptedSocket;
		if ((AcceptSocket = accept(tcp_listen, (struct sockaddr *)&client, &client_len)) == INVALID_SOCKET)
		{
			sprintf_s(temp, STR_SIZE, "accept() failed with error %d", WSAGetLastError());
			Display(temp);
			break;
		}

		/*std::thread workThread(&Server::WorkerThread, this, (LPVOID)AcceptEvent);
		workThread.detach();*/

		char* acceptedClientIp = inet_ntoa(client.sin_addr);
		SocketInfo->Socket = AcceptSocket;
		sprintf_s(temp, STR_SIZE, "Socket number %d connected: IP=%s", (int)AcceptSocket, acceptedClientIp);
		Display(temp);


		SendInitialInfo(AcceptSocket, SocketInfo);

		// Mapping the accepted client.
		ClientInformation clientInformation;
		sprintf(clientInformation.ip, "%s", acceptedClientIp);
		sprintf(clientInformation.username, "%s", "Unknown");
		mapClient[AcceptSocket] = clientInformation;

		if (WSASetEvent(AcceptEvent) == FALSE)
		{
			sprintf_s(temp, STR_SIZE, "WSASetEvent failed with error %d", WSAGetLastError());
			Display(temp);
			return;
		}

		// completion routines
		EnterCriticalSection(&CriticalSection);

		// Create a socket information structure to associate with the accepted socket.
		if ((SocketArray[EventTotal] = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			sprintf_s(temp, STR_SIZE, "GlobalAlloc() failed with error %d\n", GetLastError());
			Display(temp);
			return;
		}

		// Fill in the details of our accepted socket.
		SocketArray[EventTotal]->Socket = AcceptSocket;
		ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
		SocketArray[EventTotal]->BytesSEND = 0;
		SocketArray[EventTotal]->BytesRECV = 0;
		SocketArray[EventTotal]->DataBuf.len = BUF_SIZE;
		SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;
		SocketArray[EventTotal]->index = EventTotal;

		if ((SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal] =
			WSACreateEvent()) == WSA_INVALID_EVENT)
		{
			sprintf_s(temp, STR_SIZE, "WSACreateEvent() failed with error %d", WSAGetLastError());
			Display(temp);
			return;
		}

		DWORD Flags = 0;
		DWORD RecvBytes;
		if (WSARecv(SocketArray[EventTotal]->Socket,
			&(SocketArray[EventTotal]->DataBuf), 1, &RecvBytes, &Flags,
			&(SocketArray[EventTotal]->Overlapped), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				sprintf_s(temp, STR_SIZE, "WSARecv() failed with error %d", WSAGetLastError());
				Display(temp);
				return;
			}
		}

		EventTotal++;

		LeaveCriticalSection(&CriticalSection);

		//
		// Signal the first event in the event array to tell the worker thread to
		// service an additional event in the event array
		//
		if (WSASetEvent(AcceptEvent) == FALSE)
		{
			sprintf_s(temp, STR_SIZE, "WSASetEvent failed with error %d", WSAGetLastError());
			Display(temp);
			return;
		}
	}
}

void Server::SendInitialInfo(SOCKET socket, LPSOCKET_INFORMATION SocketInfo)
{
	for (const auto& entry : mapClient)
	{
		INFO_CLIENT infoClient;
		infoClient.header = PH_INFO_CLIENT;
		sprintf_s(infoClient.username, PACKET_STR_MAX, "%s", entry.second.username);
		sprintf_s(infoClient.ip, IP_LENGTH, "%s", entry.second.ip);
		SocketInfo->DataBuf.buf = (char*)&infoClient;
		SocketInfo->DataBuf.len = sizeof(INFO_CLIENT);
		SendTCP(socket, SocketInfo);
	}

	{
		INFO_SONG infoSong;
		infoSong.header = PH_INFO_SONG;
		infoSong.SID = 1;
		sprintf_s(infoSong.title, PACKET_STR_MAX, "%s", "Title of a song");
		sprintf_s(infoSong.artist, PACKET_STR_MAX, "%s", "Artist of a song");
		SocketInfo->DataBuf.buf = (char*)&infoSong;
		SocketInfo->DataBuf.len = sizeof(INFO_SONG);
		SendTCP(socket, SocketInfo);
	}
}

void Server::WorkThread()
{
	DWORD Flags;
	LPSOCKET_INFORMATION SI;
	DWORD Index;
	DWORD RecvBytes;

	char temp[STR_SIZE];

	// Save the accept event in the event array.

	EventArray[0] = AcceptEvent;

	while (TRUE)
	{
		if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			sprintf_s(temp, STR_SIZE, "WSAWaitForMultipleEvents failed %d", WSAGetLastError());
			return;
		}

		// If the event triggered was zero then a connection attempt was made
		// on our listening socket.

		if ((Index - WSA_WAIT_EVENT_0) == 0)
		{
			WSAResetEvent(EventArray[0]);
			continue;
		}

		SI = SocketArray[Index - WSA_WAIT_EVENT_0];
		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		// Create a socket information structure to associate with the accepted socket.

		Flags = 0;
		if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
			&(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				sprintf_s(temp, STR_SIZE, "WSARecv() failed with error %d\n", WSAGetLastError());
				Display(temp);
				return;
			}
		}

		sprintf_s(temp, STR_SIZE, "Recv from %d\n", (int)SI->Socket);
		Display(temp);
	}

	return;
}

DWORD WINAPI Server::WorkerThread(LPVOID lpParameter)
{
	char temp[STR_SIZE];

	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo;
	WSAEVENT EventArray[1];
	DWORD Index;
	DWORD RecvBytes;

	// Save the accept event in the event array.

	EventArray[0] = (WSAEVENT)lpParameter;

	while (TRUE)
	{
		// Wait for accept() to signal an event and also process WorkerRoutine() returns.

		while (TRUE)
		{
			Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

			if (Index == WSA_WAIT_FAILED)
			{
				printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
				return FALSE;
			}

			if (Index != WAIT_IO_COMPLETION)
			{
				// An accept() call event is ready - break the wait loop
				break;
			}
		}

		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		// Create a socket information structure to associate with the accepted socket.

		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return FALSE;
		}

		// Fill in the details of our accepted socket.

		SocketInfo->Socket = AcceptSocket;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->BytesSEND = 0;
		SocketInfo->BytesRECV = 0;
		SocketInfo->DataBuf.len = DATA_BUFSIZE;
		SocketInfo->DataBuf.buf = SocketInfo->Buffer;

		Flags = 0;
		if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
			&(SocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf_s(temp, STR_SIZE, "WSARecv() failed with error %d\n", WSAGetLastError());
				Display(temp);
				return FALSE;
			}
		}

		printf_s(temp, STR_SIZE, "Socket %d connected", (int)AcceptSocket);
		Display(temp);
	}

	return TRUE;
}

bool Server::SendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo)
{
	char temp[STR_SIZE];

	ZeroMemory(&SocketInfo->Overlapped, sizeof(WSAOVERLAPPED));
	SocketInfo->Overlapped.hEvent = WSACreateEvent();

	if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SocketInfo->BytesSEND,
		0, &(SocketInfo->Overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			sprintf_s(temp, STR_SIZE, "TCPControlWorker::SendToClient() WSASend() failed with: %d", WSAGetLastError());
			Display(temp);
			return false;
		}

		if (WSAWaitForMultipleEvents(1, &SocketInfo->Overlapped.hEvent, FALSE, INFINITE, FALSE) == WAIT_TIMEOUT)
		{
			sprintf_s(temp, STR_SIZE, "TCPControlWorker::SendToClient timed out");
			Display(temp);
			return false;
		}
	}

	DWORD flags;
	if (!WSAGetOverlappedResult(SocketInfo->Socket, &(SocketInfo->Overlapped),
		&SocketInfo->BytesSEND, FALSE, &flags))
	{
		sprintf_s(temp, STR_SIZE, "TCPControlWorker::SendToClient overlappedresult error %d", WSAGetLastError());
		return false;
	}

	sprintf_s(temp, STR_SIZE, "Sent %d bytes", SocketInfo->BytesSEND);
	Display(temp);

	return true;
}


void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	char temp[STR_SIZE];

	DWORD SendBytes, RecvBytes;
	DWORD Flags;

	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

	if (Error != 0)
	{
		sprintf_s(temp, STR_SIZE, "I/O operation failed with error %d", Error);
		Display(temp);
	}

	if (BytesTransferred == 0)
	{
		sprintf_s(temp, STR_SIZE, "Closing socket %d", static_cast<int>(SI->Socket));
		Display(temp);
	}

	if (Error != 0 || BytesTransferred == 0)
	{
		auto found = Server::mapClient.find(SI->Socket);
		Server::mapClient.erase(found);

		if (closesocket(SI->Socket) == SOCKET_ERROR)
		{
			sprintf_s(temp, STR_SIZE, "closesocket() failed with error %d", WSAGetLastError());
		}

		GlobalFree(SI);
		WSACloseEvent(Server::EventArray[SI->index]);

		// Cleanup SocketArray and EventArray by removing the socket event handle
		// and socket information structure if they are not at the end of the
		// arrays.

		EnterCriticalSection(&Server::CriticalSection);

		if ((SI->index) + 1 != Server::EventTotal)
			for (int i = SI->index; i < Server::EventTotal; i++)
			{
				Server::EventArray[i] = Server::EventArray[i + 1];
				Server::SocketArray[i] = Server::SocketArray[i + 1];
			}

		Server::EventTotal--;

		LeaveCriticalSection(&Server::CriticalSection);

		return;
	}

	// Check to see if the BytesRECV field equals zero. If this is so, then
	// this means a WSARecv call just completed so update the BytesRECV field
	// with the BytesTransferred value from the completed WSARecv() call.

	if (SI->BytesRECV == 0)
	{
		SI->BytesRECV = BytesTransferred;
		SI->BytesSEND = 0;
	}
	else
	{
		SI->BytesSEND += BytesTransferred;
	}

	if (SI->BytesRECV > SI->BytesSEND)
	{

		// Post another WSASend() request.
		// Since WSASend() is not gauranteed to send all of the bytes requested,
		// continue posting WSASend() calls until all received bytes are sent.

		ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

		SI->DataBuf.buf = SI->Buffer + SI->BytesSEND;
		SI->DataBuf.len = SI->BytesRECV - SI->BytesSEND;

		if (WSASend(SI->Socket, &(SI->DataBuf), 1, &SendBytes, 0,
			&(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSASend() failed with error %d\n", WSAGetLastError());
				return;
			}
		}
	}
	else
	{
		SI->BytesRECV = 0;
		SI->PacketsRECV++;

		// Now that there are no more bytes to send post another WSARecv() request.
		char temp[STR_SIZE];

		if (SI->IsFile)
		{
			std::string recvFileData(SI->Buffer, BytesTransferred);
			size_t found = recvFileData.find("EndOfPacket");
			if (found != std::string::npos)
			{
				SI->IsFile = false;
				SI->vecBuffer.push_back(recvFileData.substr(0, found));

				fwrite(SI->Buffer, 1, found, SI->file);
				fclose(SI->file);

				sprintf_s(temp, STR_SIZE, "Save a song file: %s", SI->FileName.c_str());
				Display(temp);
			}
			else
			{
				SI->vecBuffer.push_back(recvFileData);
				fwrite(SI->Buffer, 1, BytesTransferred, SI->file);
			}

			sprintf_s(temp, STR_SIZE, "default: length=%d", BytesTransferred);
			Display(temp);
		}
		else
		{
			int header;
			memcpy(&header, SI->Buffer, sizeof(int));
			switch (header)
			{
			case PH_REQ_UPLOAD_SONG:
			{
				SI->IsFile = true;
				SI->vecBuffer.clear();
				ReqUploadSong songData;
				memcpy(&songData, SI->Buffer, sizeof(ReqUploadSong));
				SI->FileName = std::string(songData.filename);

				std::string recvFileData(SI->Buffer + sizeof(ReqUploadSong), BytesTransferred - sizeof(ReqUploadSong));
				SI->vecBuffer.push_back(recvFileData);

				fopen_s(&SI->file, SI->FileName.c_str(), "wb");

				sprintf_s(temp, STR_SIZE, "Upload - file: %s, title: %s, artist: %s",
					songData.filename, songData.title, songData.artist);
				Display(temp);
			}
			break;

			case PH_REQ_DOWNLOAD_SONG:
			{
				ReqDownloadSong songData;
				memcpy(&songData, SI->Buffer, sizeof(ReqDownloadSong));

				SoundFilePacketizer packer(PACKET_SIZE);
				packer.makePacketsFromFile(songData.filename);
				long totalNumberOfPackets = packer.getTotalPackets();
				int lastPacketSize = packer.getLastPackSize();

				SI->BytesSEND = 0;
				SI->SentBytesTotal = 0;
				//send all packets except for last one
				for (int i = 0; i < totalNumberOfPackets; i++)
				{
					SI->DataBuf.buf = packer.getNextPacket();
					SI->DataBuf.len = PACKET_SIZE;
					Server::SendTCP(SI->Socket, SI);
				}

				SI->DataBuf.buf = packer.getNextPacket();
				SI->DataBuf.len = lastPacketSize;
				Server::SendTCP(SI->Socket, SI);

				char complete[] = "EndOfPacket";
				SI->DataBuf.buf = complete;
				SI->DataBuf.len = strlen(complete) + 1;
				Server::SendTCP(SI->Socket, SI);
			}
			break;

			default:
				break;
			}
		}

		Flags = 0;
		ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

		SI->DataBuf.len = DATA_BUFSIZE;
		SI->DataBuf.buf = SI->Buffer;

		if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
			&(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return;
			}
		}

		//SleepEx(INFINITE, TRUE);
	}
}