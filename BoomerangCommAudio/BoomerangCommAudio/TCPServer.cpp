#include "stdafx.h"
#include "TCPServer.h"
#include "SocketWrappers.h"

#include <WinBase.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

void TCPServer::RunServer(SOCKET& serverSock)
{
	wchar_t temp[STR_SIZE];

	InitializeCriticalSection(&CriticalSection);

	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		wsprintf(temp, L"GlobalAlloc() failed with error %d", GetLastError());
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
	if (bind(tcp_listen, (struct sockaddr *)&server, sizeof(server)) == -1)
		return;

	if (listen(tcp_listen, MAX_NUM_CLIENT) == SOCKET_ERROR)
	{
		wsprintf(temp, L"listen() failed with error %d", WSAGetLastError());
		Display(temp);
		return;
	}

	if ((EventArray[0] = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		wsprintf(temp, L"WSACreateEvent() failed with error %d", WSAGetLastError());
		Display(temp);
		return;
	}
	AcceptEvent = EventArray[0];

	wsprintf(temp, L"Listen TCP port %d", g_port);
	Display(temp);

	std::thread threadAccept(&TCPServer::AcceptFunc, this);
	threadAccept.detach();

	std::thread workThread(&TCPServer::WorkThread, this);
	workThread.detach();
}

void TCPServer::AcceptFunc()
{
	int client_len;
	struct	sockaddr_in client;
	wchar_t temp[STR_SIZE];

	EventTotal = 1;
	while (true)
	{
		client_len = sizeof(client);
		SOCKET acceptedSocket;
		if ((acceptedSocket = accept(tcp_listen, (struct sockaddr *)&client, &client_len)) == INVALID_SOCKET)
		{
			wsprintf(temp, L"accept() failed with error %d", WSAGetLastError());
			Display(temp);
			break;
		}

		char* acceptedClientIp = inet_ntoa(client.sin_addr);
		SocketInfo->Socket = acceptedSocket;
		std::wstring strIP = GetWC(acceptedClientIp);
		wsprintf(temp, L"Socket number %d connected: IP=%s", acceptedSocket, strIP.c_str());
		Display(temp);

		SendInitialInfo(acceptedSocket, SocketInfo);

		// Mapping the accepted client.
		ClientInformation clientInformation;
		sprintf(clientInformation.ip, "%s", acceptedClientIp);
		sprintf(clientInformation.username, "%s", "Unknown");
		mapClient[acceptedSocket] = clientInformation;

		// completion routines
		EnterCriticalSection(&CriticalSection);

		// Create a socket information structure to associate with the accepted socket.
		if ((SocketArray[EventTotal] = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			wsprintf(temp, L"GlobalAlloc() failed with error %d\n", GetLastError());
			Display(temp);
			return;
		}

		// Fill in the details of our accepted socket.
		SocketArray[EventTotal]->Socket = acceptedSocket;
		ZeroMemory(&(SocketArray[EventTotal]->Overlapped), sizeof(OVERLAPPED));
		SocketArray[EventTotal]->BytesSEND = 0;
		SocketArray[EventTotal]->BytesRECV = 0;
		SocketArray[EventTotal]->DataBuf.len = BUF_SIZE;
		SocketArray[EventTotal]->DataBuf.buf = SocketArray[EventTotal]->Buffer;

		if ((SocketArray[EventTotal]->Overlapped.hEvent = EventArray[EventTotal] =
			WSACreateEvent()) == WSA_INVALID_EVENT)
		{
			wsprintf(temp, L"WSACreateEvent() failed with error %d", WSAGetLastError());
			Display(temp);
			return;
		}

		DWORD Flags = 0;
		DWORD RecvBytes;
		if (WSARecv(SocketArray[EventTotal]->Socket,
			&(SocketArray[EventTotal]->DataBuf), 1, &RecvBytes, &Flags,
			&(SocketArray[EventTotal]->Overlapped), NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != ERROR_IO_PENDING)
			{
				wsprintf(temp, L"WSARecv() failed with error %d", WSAGetLastError());
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
			wsprintf(temp, L"WSASetEvent failed with error %d", WSAGetLastError());
			Display(temp);
			return;
		}
	}
}

void TCPServer::SendInitialInfo(SOCKET socket, LPSOCKET_INFORMATION SocketInfo)
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

void TCPServer::WorkThread()
{
	DWORD Flags;
	LPSOCKET_INFORMATION SI;
	DWORD Index;
	DWORD RecvBytes;

	wchar_t temp[STR_SIZE];

	// Save the accept event in the event array.

	EventArray[0] = AcceptEvent;

	while (TRUE)
	{
		//while (TRUE)
		//{
		//	Index = WSAWaitForMultipleEvents(1, &AcceptEvent, FALSE, WSA_INFINITE, TRUE);

		//	if (Index == WSA_WAIT_FAILED)
		//	{
		//		wsprintf(temp, L"WSAWaitForMultipleEvents failed with error %d", WSAGetLastError());
		//		return;
		//	}

		//	if (Index != WAIT_IO_COMPLETION)
		//	{
		//		// An accept() call event is ready - break the wait loop
		//		break;
		//	}
		//}

		if ((Index = WSAWaitForMultipleEvents(EventTotal, EventArray, FALSE,
			WSA_INFINITE, FALSE)) == WSA_WAIT_FAILED)
		{
			wsprintf(temp, L"WSAWaitForMultipleEvents failed %d", WSAGetLastError());
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

		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			wsprintf(temp, L"GlobalAlloc() failed with error %d", GetLastError());
			Display(temp);
			return;
		}

		Flags = 0;
		if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
			&(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				wsprintf(temp, L"WSARecv() failed with error %d\n", WSAGetLastError());
				Display(temp);
				return;
			}
		}

		wsprintf(temp, L"Recv from %d\n", SI->Socket);
		Display(temp);
	}

	return;
}

bool TCPServer::SendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo)
{
	wchar_t temp[STR_SIZE];

	ZeroMemory(&SocketInfo->Overlapped, sizeof(WSAOVERLAPPED));
	SocketInfo->Overlapped.hEvent = WSACreateEvent();

	if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SocketInfo->BytesSEND,
		0, &(SocketInfo->Overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			wsprintf(temp, L"TCPControlWorker::SendToClient() WSASend() failed with: ", WSAGetLastError());
			Display(temp);
			return false;
		}

		if (WSAWaitForMultipleEvents(1, &SocketInfo->Overlapped.hEvent, FALSE, INFINITE, FALSE) == WAIT_TIMEOUT)
		{
			wsprintf(temp, L"TCPControlWorker::SendToClient timed out");
			Display(temp);
			return false;
		}
	}

	DWORD flags;
	if (!WSAGetOverlappedResult(SocketInfo->Socket, &(SocketInfo->Overlapped),
		&SocketInfo->BytesSEND, FALSE, &flags))
	{
		wsprintf(temp, L"TCPControlWorker::SendToClient overlappedresult error", WSAGetLastError());
		return false;
	}

	wsprintf(temp, L"Sent %d bytes", SocketInfo->BytesSEND);
	Display(temp);

	return true;
}

TCPServer::~TCPServer()
{
	GlobalFree(SocketInfo);
}

void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	DWORD SendBytes, RecvBytes;
	DWORD Flags;

	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

	if (Error != 0)
	{
		printf("I/O operation failed with error %d\n", Error);
	}

	if (BytesTransferred == 0)
	{
		printf("Closing socket %d\n", SI->Socket);
	}

	if (Error != 0 || BytesTransferred == 0)
	{
		closesocket(SI->Socket);
		GlobalFree(SI);
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

		// Now that there are no more bytes to send post another WSARecv() request.

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
	}
}