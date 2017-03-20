#include <iostream>
#include "Client.h"
#include "SocketWrappers.h"


using namespace std;

int runClient()
{
	char buff[BUFFSIZE];
	struct sockaddr recvAddr;
	int recvAddrLen;
	int bRecv;
	SOCKET sock;
	SYSTEMTIME time;
	int count = 0;
	initializeWSA();
	sock = makeWSASocket(SOCK_DGRAM, 0);
	bindSocket(sock);
	recvAddrLen = sizeof(recvAddr);
	do {
		bRecv = recvfrom(sock, buff, BUFFSIZE, 0, &recvAddr, &recvAddrLen);
		if (bRecv < 0)
		{
			std::cerr << "RecvFrom Failed Error: "
				<< WSAGetLastError()
				<< std::endl;
			break;
		}
		GetSystemTime(&time);
		cout << "Message Recieved.\tSize: " << bRecv << " \tTimestamp: ";
		printf("%02d:%02d:%02d:%02d\n", time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);
		++count;
	} while (buff[0] != 0x04 /*ETX*/ || count < 100);
	
	closeWSA();
	return 0;
}