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
#include <windows.h>


#define BACKLOG 5
#define PORT 13456
#define PORT_STR  "13456"

bool initializeWSA(void);
boolean closeWSA(void);
SOCKET makeWSASocket(int type, int flag = WSA_FLAG_OVERLAPPED); // type == SOCK_DGRAM | SOCK_STREAM
int closeConnection(SOCKET s);
int bindSocket(SOCKET s, int port = PORT);

//Server Stuff
WSAEVENT makeWSAEvent(void);

//Client Stuff
struct addrinfo * getAddrInfo(const char * ip, const char * port, int type = SOCK_STREAM);
int connectSock(SOCKET sock, struct addrinfo * ai);
