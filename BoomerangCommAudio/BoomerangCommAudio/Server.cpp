#include "stdafx.h"
#include <Winsock2.h>
#include <chrono>
#include <stdio.h>
#include <string>
#include <vector>
#include "Common.h"
#include "Server.h"

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
	wchar_t temp[STR_SIZE];

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


	// Prepare echo server
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		wsprintf(temp, L"WSAStartup failed with error %d\n", Ret);
		Display(temp);
		return;
	}

	if ((tcp_listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		wsprintf(temp, L"socket() failed with error %d\n", WSAGetLastError());
		Display(temp);
		return;
	}

	WSAAsyncSelect(tcp_listen, g_hMainDlg, WM_TCP_SERVER_LISTEN, FD_ACCEPT | FD_CLOSE);

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = inet_addr(g_IP);
	InternetAddr.sin_port = htons(g_port);

	if (bind(tcp_listen, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		wsprintf(temp, L"bind() failed with error %d\n", WSAGetLastError());
		Display(temp);
		return;
	}

	if (listen(tcp_listen, 1))
	{
		wsprintf(temp, L"listen() failed with error %d\n", WSAGetLastError());
		Display(temp);
		return;
	}

	wsprintf(temp, L"Listen TCP port %d", g_port);
	Display(temp);

	PostMessage(g_hMainDlg, WM_CONNECT_SERVER, NULL, NULL);

}

LRESULT CALLBACK ServerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SOCKET& sock)
{
	wchar_t temp[STR_SIZE];

	if (WSAGETSELECTERROR(lParam))
	{
		wsprintf(temp, L"Socket failed with error %d", WSAGETSELECTERROR(lParam));
		Display(temp);
		CloseServer(sock);
		return 1;
	}
	switch (uMsg) {
	case WM_TCP_SERVER_LISTEN:
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_ACCEPT:
			if ((sock = accept(wParam, NULL, NULL)) == INVALID_SOCKET)
			{
				wsprintf(temp, L"accept() failed with error %d", WSAGetLastError());
				Display(temp);
				break;
			}
			SocketInfo->Socket = sock;
			wsprintf(temp, L"Socket number %d connected", sock);
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
			wsprintf(temp, L"Closing socket %d\n", sock);
			Display(temp);
			//closesocket(sock);
			break;
		}
		break;
	}
	return 0;
}


void CloseServer(SOCKET& sock)
{
	wchar_t temp[STR_SIZE];

	if (sock) {
		wsprintf(temp, L"Closing socket %d", sock);
		Display(temp);
		closesocket(sock);
	}
	if (tcp_listen) {
		wsprintf(temp, L"Closing listen socket %d", tcp_listen);
		Display(temp);
		closesocket(tcp_listen);
	}
	PostMessage(g_hMainDlg, WM_DISCONNECT_SERVER, NULL, NULL);
	GlobalFree(SocketInfo);
}

void recvTimeout()
{
	wchar_t temp[STR_SIZE] = L"";
	endTime = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(endTime - startTime).count();
	double timeGap = (double)duration;
	wsprintf(temp, L"Packet Size: %d bytes", SocketInfo->SentPacketSize);
	Display(temp);
	wsprintf(temp, L"Sent # of Packets: %d", SocketInfo->SentNumberPackets);
	Display(temp);
	wsprintf(temp, L"Sent total data: %d bytes", SocketInfo->SentBytesTotal);
	Display(temp);
	wsprintf(temp, L"Received # of Packets: %d", SocketInfo->PacketsRECV);
	Display(temp);
	wsprintf(temp, L"Received total data %d bytes", SocketInfo->BytesRECV);
	Display(temp);
	if (SocketInfo->IsFile)
	{
		std::string recvFileName(std::string("recv_" + SocketInfo->FileName));
		std::wstring strTemp(recvFileName.begin(), recvFileName.end());
		wsprintf(temp, L"File Name: %s", strTemp.c_str());
		Display(temp);
		SaveFile(recvFileName, SocketInfo->vecBuffer);
	}

	char temp2[STR_SIZE];
	sprintf(temp2, "Elapsed Time: %0.2f us. Speed: %0.2f MB/s", timeGap, duration == 0 ? 0 : (double)(SocketInfo->BytesRECV / timeGap));
	std::string strTemp(temp2);
	std::wstring strTemp2(strTemp.begin(), strTemp.end());
	Display(L"*****************************************");
	Display(strTemp2.c_str());
	Display(L"*****************************************");
	Display(L" ");

	SocketInfo->DataBuf.len = BUF_SIZE;
}

void SaveFile(const std::string &fileName, std::vector<std::string> &data)
{
	FILE* file;

	file = fopen(fileName.c_str(), "wb");
	if (file == NULL)
	{
		Display(L"Failed : Open File");
		return;
	}

	for (size_t i = 0; i < data.size(); i++) {
		fwrite(data[i].c_str(), 1, strlen(data[i].c_str()), file);
	}
	fclose(file);
}

BOOL RecvTCP(LPSOCKET_INFORMATION SocketInfo)
{
	char buffer[BUF_SIZE];
	DWORD RecvBytes;
	DWORD Flags;
	wchar_t temp[STR_SIZE];

	SocketInfo->DataBuf.buf = buffer;

	Flags = 0;
	if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			wsprintf(temp, L"WSARecv() failed with error %d\n", WSAGetLastError());
			Display(temp);
			return FALSE;
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
	return TRUE;

}

void DecodeHeader(char* header, LPSOCKET_INFORMATION SocketInfo)
{
	Display(L"****************************** Start Receiving *******************************");
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

BOOL sendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo)
{
	DWORD SendBytes;
	wchar_t temp[STR_SIZE];

	if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			wsprintf(temp, L"WSASend() failed with error %d", WSAGetLastError());
			Display(temp);
			closesocket(clientSock);
			return FALSE;
		}
	}
	else
	{
		SocketInfo->BytesSEND += SendBytes;
		wsprintf(temp, L"Sent %d bytes", SendBytes);
		Display(temp);
		return TRUE;
	}

	return TRUE;
}
