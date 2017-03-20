#include <ws2tcpip.h>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string.h>

#include "Server.h"
#include "SocketWrappers.h"

using namespace std;
int createWaitableTimer(HANDLE &timer);
int setTimer(HANDLE &timer, const LARGE_INTEGER &dueTime, const long reset = 500);
int runServer(const char * ipaddr, int numPacks, int timeinterval)
{
	char buff[BUFFSIZE] = { 0 };
	addrinfo * sendAddr;
	int bSent;
	SOCKET sock;
	HANDLE hTimer = NULL; // windows timer
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -5000000LL; // 500 MS

	if (createWaitableTimer(hTimer) > 0)
	{
		exit(1);
	}

	initializeWSA();
	sock = makeWSASocket(SOCK_DGRAM, 0);
	if(!(sendAddr = getAddrInfo(ipaddr, PORT_STR, SOCK_DGRAM))) exit(1) ;

	for (int i = 0; i < numPacks; i++)
	{
		setTimer(hTimer, dueTime);
		
		if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
		{
			printf("WaitForSingleObject failed (%d)\n", GetLastError());
			break;
		}

		bSent = sendto(sock, buff, BUFFSIZE, 0, sendAddr->ai_addr, sendAddr->ai_addrlen);
		cout << "Bytes sent: " << bSent << "\n";

	}
	buff[0] = 0x04;
	bSent = sendto(sock, buff, BUFFSIZE, 0, sendAddr->ai_addr, sendAddr->ai_addrlen);
	freeaddrinfo(sendAddr);
	closeWSA();
	return 0;
	return 0;
}

int createWaitableTimer(HANDLE &timer)
{
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (NULL == timer)
	{
		printf("CreateWaitableTimer failed (%d)\n", GetLastError());
		return 1;
	}
	return 0;
}

int setTimer(HANDLE &timer, const LARGE_INTEGER &dueTime, const long reset)
{

	if (!SetWaitableTimer(timer, &dueTime, reset, NULL, NULL, 0))
	{
		printf("SetWaitableTimer failed (%d)\n", GetLastError());
		return 2;
	}
}