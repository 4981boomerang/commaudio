
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <iostream>
#include <string.h>
#include <thread>

#include "Server.h"
#include "CBuff.h"
#include "Packetizer.h"
#include "SocketWrappers.h"

using namespace std;

void pushloop(CBuff &cbuff, SoundFilePacketizer & packer);
int createWaitableTimer(HANDLE &timer);
int setTimer(HANDLE &timer, const LARGE_INTEGER &dueTime, const long reset = 500);

/*--------------------------------------------------------------------------
-- FUNCTION: runServer
--
-- DATE: Apr. 03, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Apr/03 - comment add 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: int runServer (char * ipaddr, int numPacks, int timeinterval)
-- char * ipaddr -- the ip address to send to ( to be changed to multicast )
-- int numPacks -- the number of packets to send for testing 
-- int timeinterval --the time ( in milliseconds ) between each packet being sent
--
-- RETURNS: 
-- int representing state
--
-- NOTES:
-- this is currently procedural to be turned to object oriented 
--------------------------------------------------------------------------*/
int runServer(const char * ipaddr, int numPacks, int timeinterval)
{
	const char * file = "FILE_IN_HERE";
	//const char * file = "C:\\Users\\Eva\\Documents\\CST\\Semester4\\comp4985\\assignments\\a4\\a.txt";
	
	char buff[BUFFSIZE]{ 0 };
	addrinfo * sendAddr;
	int bSent;
	SOCKET sock;
	HANDLE hTimer = NULL; // windows timer
	LARGE_INTEGER dueTime;
	string str = "";
	SoundFilePacketizer packer;
	CBuff cbuff;
	long numP;

	packer.makePacketsFromFile(file);
	numP = packer.getTotalPackets();
	thread popthread(pushloop, std::ref(cbuff), std::ref(packer));

	dueTime.QuadPart = -1200000LL; // 120 MS
	if ( createWaitableTimer(hTimer) > 0)
	{
		exit(1);
	}

	initializeWSA();
	sock = makeWSASocket(SOCK_DGRAM, 0);
	if(!(sendAddr = getAddrInfo(ipaddr, PORT_STR, SOCK_DGRAM))) exit(1) ;

	for (int i = 0; i < numP; i++)
	{
		setTimer(hTimer, dueTime);
		if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
		{
			printf("WaitForSingleObject failed (%d)\n", GetLastError());
			break;
		}
		str = cbuff.pop();
		memcpy(buff, str.c_str(), str.length());
		bSent = sendto(sock, buff, BUFFSIZE, 0, sendAddr->ai_addr, sendAddr->ai_addrlen);
		cout << "Bytes sent: " << bSent << "\n";
	}

	buff[0] = 0x04;
	bSent = sendto(sock, buff, BUFFSIZE, 0, sendAddr->ai_addr, sendAddr->ai_addrlen);
	freeaddrinfo(sendAddr);
	closeWSA();
	return 0;
}

/*--------------------------------------------------------------------------
-- FUNCTION: CreateWaitableTimer
--
-- DATE: Apr. 4, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Apr/4 - Creted comment 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: intq CreateWaitableTimer (HANLDE &timer)
-- HANLDE &timer -- the handle to the windows timer object
--
-- RETURNS: 
-- int representing state od create ( 0 for success )
--
-- NOTES:
-- This creares a windows timer object, unset and not running
--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------
-- FUNCTION: setTimer
--
-- DATE: Apr. 04, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Apr/04 - Created Comment 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: int setTimer (HANDLE &timer, const LARGE_INTEGER &dueTime, const long reset)
-- HANDLE &timer -- handle to the timer to set
-- const LARGE_INTEGER &dueTime -- a long long int that represents the timer in (100th of a nanosecond)
-- const long reset -- ( default 500 ), actually... im not sure what this is for 
--
-- RETURNS: 
-- int representing state of setting ( 0 is success)
--
-- NOTES:
-- Timer must be reset with every use! 
--------------------------------------------------------------------------*/
int setTimer(HANDLE &timer, const LARGE_INTEGER &dueTime, const long reset)
{

	if (!SetWaitableTimer(timer, &dueTime, reset, NULL, NULL, 0))
	{
		printf("SetWaitableTimer failed (%d)\n", GetLastError());
		return 1;
	}
	return 0;
}

/*--------------------------------------------------------------------------
-- FUNCTION: pushloop
--
-- DATE: apr. 04, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/apr/04 - created Comment 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void pushloop (CBuff &cbuff, SoundFilePacketizer &packer)
-- CBuff &cbuff, SoundFilePacketizer &packer the packetizer that will be read from 
--
-- NOTES:
-- This is a temp loop for the testing of CBuff's sempahores in a threaded 
- -environment. This will be transferred in ot the server class
-- as apppropriate
--------------------------------------------------------------------------*/
void pushloop(CBuff &cbuff, SoundFilePacketizer & packer)
{
	string str;
	long ttl = packer.getTotalPackets();

	do
	{
		str = packer.getNextPacket();
		if (str.empty())
			break;
		cbuff.push_back(str);
	} while (!str.empty());

	return;
}