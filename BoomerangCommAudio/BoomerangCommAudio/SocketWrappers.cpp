#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "SocketWrappers.h"

//** initialize WSA DLL Wrapper --- Feb 3rd
bool initializeWSA(void)
{
	WSADATA wsadata;
	int result;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if ((result = WSAStartup(wVersionRequested, &wsadata)) != 0)
	{
		std::cerr << "Failed on WSAStartup; error:"
			<< WSAGetLastError()
			<< std::endl;
		return false;
	}
	return true;
}

//** close WSA DLL Wrapper --- Feb 3rd
bool closeWSA(void)
{
	if (WSACleanup() == SOCKET_ERROR)
	{
		std::cerr << "Call to cleanup WSA failed; error: "
			<< WSAGetLastError()
			<< std::endl;
		return false;
	}
	return true;
}

//** close socket / shutdown Connection Wrapper --- Feb 3rd
int closeConnection(SOCKET s)
{
	if (shutdown(s, SD_BOTH) == SOCKET_ERROR)
	{
		std::cerr << "Call to shutdown socket failed; error: "
			<< WSAGetLastError()
			<< std::endl;
	}
	if (closesocket(s) == SOCKET_ERROR)
	{
		std::cerr << "Call to close socket failed; error: "
			<< WSAGetLastError()
			<< std::endl;
	}
	return 0;
}

//** make socket Connection Wrapper --- Feb 3rd
SOCKET makeWSASocket(int type, int flag)
{
	SOCKET sock;
	if ((sock = WSASocket(PF_INET, type, 0, NULL, 0, flag))
		== INVALID_SOCKET)
	{
		std::cerr << "Call to make socket failed; error: "
			<< WSAGetLastError()
			<< std::endl;
		return -1;
	}
	return sock;
}

//** make socket bind Wrapper --- Feb 5th
int bindSocket(SOCKET s, int port)
{
	SOCKADDR_IN addrInfo;
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	addrInfo.sin_port = htons(port);

	if (bind(s, (PSOCKADDR)&addrInfo, sizeof(addrInfo)) == SOCKET_ERROR)
	{
		std::cerr << "Call to bind socket failed; error: "
			<< WSAGetLastError()
			<< std::endl;
		return -3;
	}
	return 0;
}
/******** SERVER SIDE OPERATIONS ********/


//** make event for async handling wrapper --- Feb 18th
WSAEVENT makeWSAEvent(void)
{
	WSAEVENT evnt;
	if ((evnt = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		std::cerr << "WSACreateEvent failed with error : "
			<< WSAGetLastError()
			<< std::endl;
		return NULL;
	}
	return evnt;
}

/******** CLIENT SIDE OPERATIONS ********/

//Get Addr Info Wrapper
addrinfo * getAddrInfo(const char * ip, const char * port, int type)
{
	int status;
	struct addrinfo serverhints;
	struct addrinfo *addrResults;

	memset(&serverhints, 0, sizeof(addrinfo));
	serverhints.ai_socktype = type; // SOCK STREAM by defaults
	serverhints.ai_flags = AI_PASSIVE;
	serverhints.ai_family = AF_INET; // IPV4 only for now

	if ((status = getaddrinfo(ip, port, &serverhints, &addrResults)) != 0) {
		std::cerr << "GetAddrInfo failed with error : "
			<< WSAGetLastError()
			<< std::endl;
		return nullptr;
	}
	return addrResults;
}

int connectSock(SOCKET sock, addrinfo * ai)
{
	struct addrinfo *p;

	for (p = ai; p != NULL; p = p->ai_next)
	{
		if (connect(sock, p->ai_addr, (int)p->ai_addrlen) == 0)
			return 0;
	}
	std::cerr << "connect failed with error : " << WSAGetLastError() << std::endl;
	closesocket(sock);
	return -1;
}

int waitSingleEvent(WSAEVENT event)
{
	if (WaitForSingleObject(event, INFINITE) == WAIT_FAILED)
	{
		std::cerr << "WaitForSingleObject failed with error : "
			<< WSAGetLastError()
			<< std::endl;
		return -1;
	}
	return 0;
}


