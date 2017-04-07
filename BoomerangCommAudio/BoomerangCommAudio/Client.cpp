#include "stdafx.h"
#include <Winsock2.h>
#include <stdio.h>
#include "Common.h"
#include "Client.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4996)

void ConnectClient(SOCKET& clientSock)
{
	DWORD Ret;
	WSADATA wsaData;
	wchar_t temp[STR_SIZE];

	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		wsprintf(temp, L"WSAStartup failed with error %d", Ret);
		Display(temp);
		return;
	}

	SOCKADDR_IN InternetAddr;

	if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		wsprintf(temp, L"socket() failed with error %d", WSAGetLastError());
		Display(temp);
		return;
	}

	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = inet_addr(g_IP);
	InternetAddr.sin_port = htons(g_port);

	WSAAsyncSelect(clientSock, g_hMainDlg, WM_CLIENT_TCP, FD_CONNECT | FD_WRITE | FD_CLOSE);

	if ((Ret == WSAConnect(clientSock, (struct sockaddr *)&InternetAddr, sizeof(InternetAddr), 0, 0, 0, NULL)) != 0) {
		wsprintf(temp, L"WSAConnect() failed with error %d", WSAGetLastError());
		Display(temp);
		return;
	}
	std::string strTemp(g_IP);
	std::wstring strTemp2(strTemp.begin(), strTemp.end());
	wsprintf(temp, L"Connecting to TCP server. IP: %s, Port: %d", strTemp2.c_str(), g_port);
	Display(temp);
	PostMessage(g_hMainDlg, WM_CONNECT_CLIENT, NULL, NULL);
}

void SendPacket(SOCKET& clientSock)
{
	LPSOCKET_INFORMATION SocketInfo;
	char *header;
	wchar_t temp[STR_SIZE];

	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		wsprintf(temp, L"GlobalAlloc() failed with error %d\n", GetLastError());
		Display(temp);
		return;
	}
	SocketInfo->Socket = clientSock;

	header = getHeader(g_packetSize, g_packetTimes, g_packetSize*g_packetTimes, 0, "");
	SocketInfo->DataBuf.buf = header;
	SocketInfo->DataBuf.len = strlen(header) + 1;

	char* packet = (char*)malloc(sizeof(char)*g_packetSize);
	DummyPacket(packet);

	Display(L"");
	Display(L"------------------------- Start Sending -------------------------");

	if (!sendTCP(SocketInfo, clientSock))
		return;

	SocketInfo->BytesSEND = 0;
	SocketInfo->DataBuf.buf = packet;
	SocketInfo->DataBuf.len = g_packetSize;
	for (int i = 0; i < g_packetTimes; i++)
	{
		if (!sendTCP(SocketInfo, clientSock))
			break;
	}

	Display(L"*****************************************");
	wsprintf(temp, L"Total Sent Data: %d bytes\n", SocketInfo->BytesSEND);
	Display(temp);
	Display(L"*****************************************");
	Display(L"");
	GlobalFree(SocketInfo);
	free(packet);
	free(header);
}

LRESULT CALLBACK ClientProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SOCKET& sock)
{
	wchar_t temp[STR_SIZE];

	if (WSAGETSELECTERROR(lParam))
	{
		wsprintf(temp, L"Socket failed with error %d", WSAGETSELECTERROR(lParam));
		Display(temp);
		closeClient(sock);
		return 1;
	}

	switch (uMsg) {
	case WM_CLIENT_TCP:
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_CONNECT:
			break;
		case FD_WRITE:
			break;
		case FD_CLOSE:
			closeClient(sock);
			break;
		}
		break;
	case WM_CLIENT_UDP:
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_CLOSE:
			closeClient(sock);
			break;
		}
		break;
	}
	return 0;
}

BOOL sendTCP(LPSOCKET_INFORMATION SocketInfo, SOCKET& clientSock)
{
	DWORD SendBytes;
	wchar_t temp[STR_SIZE];

	if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			wsprintf(temp, L"WSASend() failed with error %d", WSAGetLastError());
			Display(temp);
			closeClient(clientSock);
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

char* getHeader(int packetSize, int numPacket, int totalSize, int isFile, std::string filename)
{
	char* buff = (char *)malloc(STR_SIZE);
	sprintf(buff, "HEADER:%d:%d:%d:%d:%s", packetSize, numPacket, totalSize, isFile, filename.c_str());
	return buff;
}

void closeClient(SOCKET& clientSock)
{
	wchar_t temp[STR_SIZE];
	wsprintf(temp, L"Closing socket %d\n", clientSock);
	Display(temp);
	closesocket(clientSock);
	PostMessage(g_hMainDlg, WM_DISCONNECT_CLIENT, NULL, NULL);
}

void DummyPacket(char* packet)
{

	int i;
	for (i = 0; i < g_packetSize; i++)
		packet[i] = 'x';
	packet[i] += '\0';
}

void sendFile(SOCKET& clientSocket)
{
	char *packet;
	FILE *file;
	LPSOCKET_INFORMATION SocketInfo;
	DWORD nread;
	char *header;
	DWORD fileSize;
	DWORD numPackets;
	std::string fileName(g_filename);
	wchar_t temp[STR_SIZE];


	packet = (char*)malloc(sizeof(char)*g_packetSize);

	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		wsprintf(temp, L"GlobalAlloc() failed with error %d\n", GetLastError());
		Display(temp);
		return;
	}
	SocketInfo->Socket = clientSocket;

	file = fopen(g_filename, "rb+");
	if (file == NULL)
	{
		Display(L"Failed : Open File");
		return;
	}
	fseek(file, 0, SEEK_END);
	fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	numPackets = fileSize / g_packetSize;
	numPackets += (fileSize % g_packetSize != 0) ? 1 : 0;
	fileName = fileName.substr(fileName.find_last_of("\\") + 1);
	header = getHeader(g_packetSize, numPackets, fileSize, 1, fileName);
	SocketInfo->DataBuf.buf = header;
	SocketInfo->DataBuf.len = strlen(header) + 1;

	Display(L"");
	Display(L"-------------------------- Start Sending --------------------------");

	if (!sendTCP(SocketInfo, clientSocket))
		return;

	SocketInfo->BytesSEND = 0;
	//read file and send until the file finished
	while (!feof(file)) {

		//read file
		nread = fread(packet, sizeof(char), g_packetSize, file);
		SocketInfo->DataBuf.buf = packet;
		SocketInfo->DataBuf.len = nread;

		if (!sendTCP(SocketInfo, clientSocket))
			return;
	}

	Display(L"*****************************************");
	wsprintf(temp, L"Total Sent Data: %d bytes\n", SocketInfo->BytesSEND);
	Display(temp);
	Display(L"*****************************************");
	Display(L"");
	free(packet);
	free(header);
	fclose(file);
	GlobalFree(SocketInfo);
}
