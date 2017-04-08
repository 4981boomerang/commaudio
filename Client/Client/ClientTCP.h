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
#include "Microphone.h"

#define DEFAULT_PORT	5000	
#define WM_SOCKET		104
#define PACKET_SIZE		1024
#define MAX_THREADS		10
#define STR_MAX_SIZE	128
#define STR_NAME		128

#define DEFAULT_IP		"192.168.0.22"
#define TEST_FILE		"06 - Little Wing.flac"


#define SONG_UPDATE				1
#define CLIENT_UPDATE			2
#define SONG_REQUEST			3
#define CLIENT_MIC_CONNECTION	4


//deprecated
typedef struct {
	int header;
	union {
		int numOfSongs;
		int numOfClients;
		int SID;		
	};
} ControlMessage;

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




