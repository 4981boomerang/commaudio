#include "stdafx.h"
#include <Winsock2.h>
#include <WinBase.h>
#include <chrono>
#include <stdio.h>
#include <string>
#include <vector>
#include <thread>
#include "Common.h"
#include "Server.h"
#include "SocketWrappers.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

using namespace std::chrono;

SOCKET tcp_listen;
LPSOCKET_INFORMATION SocketInfo;
RecvTimer recvTimer;
high_resolution_clock::time_point startTime;
high_resolution_clock::time_point endTime;

void RunServer(SOCKET& serverSock)
{
	DWORD Ret;
	SOCKADDR_IN InternetAddr;
	WSADATA wsaData;
	char temp[STR_SIZE];

	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		sprintf_s(temp, "GlobalAlloc() failed with error %d", GetLastError());
		Display(temp);
		return;
	}

	SocketInfo->BytesRECV = 0;
	SocketInfo->BytesSEND = 0;
	SocketInfo->PacketsRECV = 0;
	SocketInfo->DataBuf.len = BUF_SIZE;
	//SocketInfo->Overlapped.hEvent = makeWSAEvent();

	if (!initializeWSA())
		return;

	// create a socket.
	//if ((tcp_listen = makeWSASocket(SOCK_STREAM, 0)) == INVALID_SOCKET)
	if ((tcp_listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return;

	struct sockaddr_in server;
	memset((char *)&server, 0, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(g_port);
	server.sin_addr.s_addr = htonl(INADDR_ANY); // Accept connections from any client

	// Bind an address to the socket
	//bindSocket(tcp_listen, htons(g_port));
	if (bind(tcp_listen, (struct sockaddr *)&server, sizeof(server)) == -1)
		return;

	if (listen(tcp_listen, MAX_NUM_CLIENT) == SOCKET_ERROR)
	{
		sprintf_s(temp, "listen() failed with error %d", WSAGetLastError());
		Display(temp);
		return;
	}

	sprintf_s(temp, "Listen TCP port %d", g_port);
	Display(temp);

	std::thread threadAccept(AcceptFunc);
	threadAccept.detach();


	// Prepare echo server
	/*if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		sprintf_s(temp, "WSAStartup failed with error %d\n", Ret);
		Display(temp);
		return;
	}

	if ((tcp_listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		sprintf_s(temp, "socket() failed with error %d\n", WSAGetLastError());
		Display(temp);
		return;
	}

	WSAAsyncSelect(tcp_listen, g_hMainDlg, WM_TCP_SERVER_LISTEN, FD_ACCEPT | FD_CLOSE);

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = inet_addr(g_IP);
	InternetAddr.sin_port = htons(g_port);

	if (bind(tcp_listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		sprintf_s(temp, "bind() failed with error %d\n", WSAGetLastError());
		Display(temp);
		return;
	}

	if (listen(tcp_listen, 1))
	{
		sprintf_s(temp, "listen() failed with error %d\n", WSAGetLastError());
		Display(temp);
		return;
	}

	sprintf_s(temp, "Listen TCP port %d", g_port);
	Display(temp);

	PostMessage(g_hMainDlg, WM_CONNECT_SERVER, NULL, NULL);
	*/

}

void AcceptFunc()
{
	int client_len;
	struct	sockaddr_in client;
	char temp[STR_SIZE];

	while (true)
	{
		client_len = sizeof(client);
		SOCKET acceptedSocket;
		if ((acceptedSocket = accept(tcp_listen, (struct sockaddr *)&client, &client_len)) == INVALID_SOCKET)
		{
			sprintf_s(temp, "accept() failed with error %d", WSAGetLastError());
			Display(temp);
			break;
		}

		char* acceptedClientIp = inet_ntoa(client.sin_addr);
		SocketInfo->Socket = acceptedSocket;
		sprintf_s(temp, "Socket number %d connected: IP=%s", acceptedSocket, acceptedClientIp);
		Display(temp);

		INFO_SONG infoSong;
		infoSong.header = PH_INFO_SONG;
		infoSong.SID = 1;
		sprintf_s(infoSong.title, PACKET_STR_MAX, "%s", "Title of a song");
		sprintf_s(infoSong.artist, PACKET_STR_MAX, "%s", "Artist of a song");
		SocketInfo->DataBuf.buf = (char*)&infoSong;
		SocketInfo->DataBuf.len = sizeof(INFO_SONG);
		sendTCP(acceptedSocket, SocketInfo);

		INFO_CLIENT infoClient;
		infoClient.header = PH_INFO_CLIENT;
		sprintf_s(infoClient.username, PACKET_STR_MAX, "%s", "luxes");
		sprintf_s(infoClient.ip, IP_LENGTH, "%s", "192.168.0.22");
		SocketInfo->DataBuf.buf = (char*)&infoClient;
		SocketInfo->DataBuf.len = sizeof(INFO_CLIENT);
		sendTCP(acceptedSocket, SocketInfo);
	}
}

LRESULT CALLBACK ServerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SOCKET& sock)
{
	char temp[STR_SIZE];

	if (WSAGETSELECTERROR(lParam))
	{
		sprintf_s(temp, "Socket failed with error %d", WSAGETSELECTERROR(lParam));
		Display(temp);
		//CloseServer(sock);
		return 1;
	}
	switch (uMsg) {
	case WM_TCP_SERVER_LISTEN:
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			if ((sock = accept(wParam, NULL, NULL)) == INVALID_SOCKET)
			{
				sprintf_s(temp, "accept() failed with error %d", WSAGetLastError());
				Display(temp);
				break;
			}
			SocketInfo->Socket = sock;
			sprintf_s(temp, "Socket number %d connected", sock);
			Display(temp);

			WSAAsyncSelect(sock, hwnd, WM_SOCKET_TCP, FD_READ | FD_CLOSE);

			{	// TODO: send the # of songs(control msg), the info of all songs separately, # of clients(control msg), info of all clients separately.
				/*CONTROL_MSG msgNumSongs;
				msgNumSongs.header = PH_NUM_OF_SONGS;
				msgNumSongs.msg = { 1 };
				SocketInfo->DataBuf.buf = (char*)&msgNumSongs;
				SocketInfo->DataBuf.len = sizeof(CONTROL_MSG);
				sendTCP(sock, SocketInfo);*/

				INFO_SONG infoSong;
				infoSong.header = PH_INFO_SONG;
				infoSong.SID = 1;
				sprintf_s(infoSong.title, PACKET_STR_MAX, "%s", "Title of a song");
				sprintf_s(infoSong.artist, PACKET_STR_MAX, "%s", "Artist of a song");
				SocketInfo->DataBuf.buf = (char*)&infoSong;
				SocketInfo->DataBuf.len = sizeof(INFO_SONG);
				sendTCP(sock, SocketInfo);

				/*CONTROL_MSG msgNumClients;
				msgNumClients.header = PH_NUM_OF_CLIENT;
				msgNumClients.msg = { 1 };
				SocketInfo->DataBuf.buf = (char*)&msgNumClients;
				SocketInfo->DataBuf.len = sizeof(CONTROL_MSG);
				sendTCP(sock, SocketInfo);*/

				INFO_CLIENT infoClient;
				infoClient.header = PH_INFO_CLIENT;
				sprintf_s(infoClient.username, PACKET_STR_MAX, "%s", "luxes");
				sprintf_s(infoClient.ip, IP_LENGTH, "%s", "192.168.0.22");
				SocketInfo->DataBuf.buf = (char*)&infoClient;
				SocketInfo->DataBuf.len = sizeof(INFO_CLIENT);
				sendTCP(sock, SocketInfo);
			}

			break;
		case FD_CLOSE:
			//CloseServer(sock);
			break;
		}
		break;
	case WM_SOCKET_TCP:
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			if (!RecvTCP(SocketInfo))
				break;
			break;
		case FD_CLOSE:
			sprintf_s(temp, "Closing socket %d\n", sock);
			Display(temp);
			closesocket(sock);
			break;
		}
		break;
	}
	return 0;
}


void CloseServer(SOCKET& sock)
{
	char temp[STR_SIZE];

	if (sock) {
		sprintf_s(temp, "Closing socket %d", sock);
		Display(temp);
		closesocket(sock);
	}
	if (tcp_listen) {
		sprintf_s(temp, "Closing listen socket %d", tcp_listen);
		Display(temp);
		closesocket(tcp_listen);
	}
	PostMessage(g_hMainDlg, WM_DISCONNECT_SERVER, NULL, NULL);
	GlobalFree(SocketInfo);
}

void recvTimeout()
{
	char temp[STR_SIZE] = "";
	endTime = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(endTime - startTime).count();
	double timeGap = (double)duration;
	sprintf_s(temp, "Packet Size: %d bytes", SocketInfo->SentPacketSize);
	Display(temp);
	sprintf_s(temp, "Sent # of Packets: %d", SocketInfo->SentNumberPackets);
	Display(temp);
	sprintf_s(temp, "Sent total data: %d bytes", SocketInfo->SentBytesTotal);
	Display(temp);
	sprintf_s(temp, "Received # of Packets: %d", SocketInfo->PacketsRECV);
	Display(temp);
	sprintf_s(temp, "Received total data %d bytes", SocketInfo->BytesRECV);
	Display(temp);
	if (SocketInfo->IsFile)
	{
		std::string recvFileName(std::string("recv_" + SocketInfo->FileName));
		std::wstring strTemp(recvFileName.begin(), recvFileName.end());
		sprintf_s(temp, "File Name: %s", strTemp.c_str());
		Display(temp);
		SaveFile(recvFileName, SocketInfo->vecBuffer);
	}

	char temp2[STR_SIZE];
	sprintf_s(temp2, "Elapsed Time: %0.2f us. Speed: %0.2f MB/s", timeGap, duration == 0 ? 0 : (double)(SocketInfo->BytesRECV / timeGap));
	std::string strTemp(temp2);
	Display("*****************************************");
	Display(strTemp.c_str());
	Display("*****************************************");
	Display(" ");

	SocketInfo->DataBuf.len = BUF_SIZE;
}

void SaveFile(const std::string &fileName, std::vector<std::string> &data)
{
	FILE* file;

	file = fopen(fileName.c_str(), "wb");
	if (file == NULL)
	{
		Display("Failed : Open File");
		return;
	}

	for (size_t i = 0; i < data.size(); i++) {
		fwrite(data[i].c_str(), 1, strlen(data[i].c_str()), file);
	}
	fclose(file);
}

bool RecvTCP(LPSOCKET_INFORMATION SocketInfo)
{
	char buffer[BUF_SIZE];
	DWORD RecvBytes;
	DWORD Flags;
	char temp[STR_SIZE];

	SocketInfo->DataBuf.buf = buffer;

	Flags = 0;
	if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			sprintf_s(temp, "WSARecv() failed with error %d\n", WSAGetLastError());
			Display(temp);
			return false;
		}
	}
	else // No error so update the byte count
	{
		recvTimer.start();
		std::string str(SocketInfo->DataBuf.buf);
		bool exists = (str.find("HEADER") != std::string::npos);
		if (exists) {
			DecodeHeader(buffer, SocketInfo);
			SocketInfo->BytesRECV = 0;
			SocketInfo->PacketsRECV = 0;
		}
		else {
			if (SocketInfo->IsFile)
				SocketInfo->vecBuffer.push_back(std::string(buffer, RecvBytes));
			SocketInfo->BytesRECV += RecvBytes;
			SocketInfo->PacketsRECV++;
		}
	}
	return true;

}

void DecodeHeader(char* header, LPSOCKET_INFORMATION SocketInfo)
{
	Display("****************************** Start Receiving *******************************");
	startTime = high_resolution_clock::now();
	std::string strHeader(header);
	std::vector<std::string> params;

	std::string::size_type i = 0;
	std::string::size_type j = strHeader.find(':');

	while (j != std::string::npos) 
	{
		params.push_back(strHeader.substr(i, j - i));
		i = ++j;
		j = strHeader.find(':', j);

		if (j == std::string::npos)
			params.push_back(strHeader.substr(i, strHeader.length()));
	}

	SocketInfo->SentPacketSize = atoi(params[1].c_str());
	SocketInfo->SentNumberPackets = atoi(params[2].c_str());
	SocketInfo->SentBytesTotal = atoi(params[3].c_str());
	SocketInfo->IsFile = atoi(params[4].c_str());
	if (SocketInfo->IsFile)
		SocketInfo->FileName = params[5];
	SocketInfo->vecBuffer.clear();

	SocketInfo->DataBuf.len = SocketInfo->SentPacketSize;

}

bool sendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo)
{
	char temp[STR_SIZE];

	ZeroMemory(&SocketInfo->Overlapped, sizeof(WSAOVERLAPPED));
	SocketInfo->Overlapped.hEvent = WSACreateEvent();

	if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SocketInfo->BytesSEND,
		0, &(SocketInfo->Overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			sprintf_s(temp, "TCPControlWorker::SendToClient() WSASend() failed with: %d", WSAGetLastError());
			Display(temp);
			return false;
		}

		if (WSAWaitForMultipleEvents(1, &SocketInfo->Overlapped.hEvent, FALSE, INFINITE, FALSE) == WAIT_TIMEOUT)
		{
			sprintf_s(temp, "TCPControlWorker::SendToClient timed out");
			Display(temp);
			return false;
		}
	}

	DWORD flags;
	if (!WSAGetOverlappedResult(SocketInfo->Socket, &(SocketInfo->Overlapped),
		&SocketInfo->BytesSEND, FALSE, &flags))
	{
		sprintf_s(temp, "TCPControlWorker::SendToClient overlappedresult error %d", WSAGetLastError());
		return false;
	}

	sprintf_s(temp, "Sent %d bytes", SocketInfo->BytesSEND);
	Display(temp);

	return true;

	/*
	DWORD SendBytes;
	char temp[STR_SIZE];

	if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			sprintf_s(temp, "WSASend() failed with error %d", WSAGetLastError());
			Display(temp);
			closesocket(clientSock);
			return FALSE;
		}
	}
	else
	{
		SocketInfo->BytesSEND += SendBytes;
		sprintf_s(temp, "Sent %d bytes", SendBytes);
		Display(temp);
		return TRUE;
	}

	return TRUE;
	*/
}
