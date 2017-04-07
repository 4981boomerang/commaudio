#pragma once

#include <string>

void ConnectClient(SOCKET& clientSock);
void SendPacket(SOCKET& clientSock);
BOOL sendTCP(LPSOCKET_INFORMATION SocketInfo, SOCKET& clientSock);
char* getHeader(int packetSize, int numPacket, int totalSize, int isFile, std::string filename);
void closeClient(SOCKET& clientSock);
LRESULT CALLBACK ClientProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, SOCKET& sock);
void DummyPacket(char* packet);
void sendFile(SOCKET& clientSocket);
