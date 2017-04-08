#pragma once

#include "resource.h"
#include "Timer.h"

void RunServer(SOCKET& serverSock);
void AcceptFunc();
LRESULT CALLBACK ServerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SOCKET& sock);
void CloseServer(SOCKET& sock);
void recvTimeout();
void SaveFile(const std::string &fileName, std::vector<std::string> &data);
BOOL RecvTCP(LPSOCKET_INFORMATION SocketInfo);
void DecodeHeader(char* header, LPSOCKET_INFORMATION SocketInfo);
BOOL sendTCP(SOCKET& clientSock, LPSOCKET_INFORMATION SocketInfo);

typedef Timer<&recvTimeout, 500> RecvTimer;