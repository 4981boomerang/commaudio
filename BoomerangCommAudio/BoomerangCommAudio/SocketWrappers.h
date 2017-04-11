/*------------------------------------------------------------------------------
-- SOURCE FILE: wsawrapper.h - The Wrapper class
--
-- PROGRAM: COMP4985 A2
--
-- FUNCTIONS:
--    boolean initializeWSA(void);
--    SOCKET makeWSAOverlapSocket(int type);
--    int closeConnection(SOCKET s);
--    boolean closeWSA(void);

--    int getHostByName(char * res, int size);
--    int getHostIP(char * dest);
--    int listenSocket(SOCKET s);
--    int bindSocket(SOCKET s, int port);
--    SOCKET acceptConnect(SOCKET s);
--    WSAEVENT makeWSAEvent(void);
--    struct addrinfo * getAddrInfo(const char * ip, const char * port, int type = SOCK_STREAM);
--    int connectSock(SOCKET sock, struct addrinfo * ai);
--    void * globalAlloc(int size);

--
-- DATE: MM. DD, 2016
--
-- REVISIONS:
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION

-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- A whole family of WSA wrappers!
------------------------------------------------------------------------------*/

#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#define BACKLOG 5
#define PORT 13456
#define PORT_STR  "13456"

bool initializeWSA(void);
bool closeWSA(void);
SOCKET makeWSASocket(int type, int flag = WSA_FLAG_OVERLAPPED); // type == SOCK_DGRAM | SOCK_STREAM
int closeConnection(SOCKET s);
int bindSocket(SOCKET s, int port = PORT);

//Server Stuff
WSAEVENT makeWSAEvent(void);

//Client Stuff
struct addrinfo * getAddrInfo(const char * ip, const char * port, int type = SOCK_STREAM);
int connectSock(SOCKET sock, struct addrinfo * ai);

/** inline functions ** /
/*------------------------------------------------------------------------------
-- FUNCTION: setSockOpt
--
-- DATE: Mar. 22, 2017
--
-- REVISIONS:
-- 1.0  - EY - Description
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: inline int setSockOpt(int sock, int level, int cmd, void * req, const int reqlen)
-- sock  -- the socket to set the resuse on
-- level -- socket level
-- cmd -- command to put on the socket
-- req -- the request structure
-- the request struct length
--
-- RETURNS:
-- int  -- >0 represents success; returns number of bytes read
--
-- NOTES:
-- sets the socket options
--------------------------------------------------------------------------*/
inline int setSockOpt(int sock, int level, int cmd, void * req, const int reqlen)
{
	if (setsockopt(sock, level, cmd,(char *)req, reqlen) < 0)
	{
		perror("setsockopt() error");
		exit(1);
	}
	return 0;
}

inline int addToMcastGroup(int sock, ip_mreq & mreq)
{
	return setSockOpt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(ip_mreq));
}

inline int rmvFromMcastGroup(int sock, ip_mreq & mreq)
{
	return setSockOpt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(ip_mreq));
}