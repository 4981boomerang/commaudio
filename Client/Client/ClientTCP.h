#ifndef CLIENTTCP_H
#define CLIENTTCP_H

#define STRICT
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _UNICODE

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <streambuf>
#include "MenuSettings.h"
#include <tchar.h>
#include <strsafe.h>
#include <vector>
#include <ws2tcpip.h>
#include "Microphone.h"
#include "Packetizer.h"
//#include "../../Packetizer/Packetizer/CBuff.h"


#define DEFAULT_PORT	5000	
#define WM_SOCKET		104
#define PACKET_SIZE		1024
#define MAX_THREADS		10
#define STR_MAX_SIZE	128
#define STR_NAME		128

//#define DEFAULT_IP		"192.168.0.22"
#define DEFAULT_IP		"10.211.55.3"
#define TEST_FILE		"06 - Little Wing.flac"
#define TEST_ARTIST		"Jimi Hendrix"
#define TEST_TITLE		"Little Wing"


#define SONG_UPDATE				1
#define CLIENT_UPDATE			2
#define SONG_DOWNLOAD			3
#define CLIENT_MIC_CONNECTION	4

typedef struct {
	int header;
	char username[STR_NAME];
	char ip[STR_NAME];
} ClientData;

typedef struct {
	int header;
	int SID;
	char title[STR_MAX_SIZE];
	char artist[STR_MAX_SIZE];
} SongData;


LRESULT CALLBACK Idle(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI recvCommand(LPVOID lpParam);
DWORD WINAPI connect(LPVOID lpParam);
DWORD WINAPI music(LPVOID lpParam);
DWORD WINAPI uploadFile(LPVOID lpParam);
DWORD WINAPI downloadFile(LPVOID lpParam);

static TCHAR Name[] = TEXT("COMM Audio Client");
HWND hwnd;
HINSTANCE mainInst;
SOCKET Socket = NULL;
HANDLE  hThreadArray[MAX_THREADS];
DWORD dwThreadIdArray[MAX_THREADS];
std::vector<SongData> songs;
std::vector<ClientData> clients;
int numberOfThreads;
SoundFilePacketizer packer;
HANDLE fileInputHandle;

#endif