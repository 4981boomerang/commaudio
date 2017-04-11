#pragma once

#include <Winsock2.h>
#include <windowsx.h>
#include <string>
#include <vector>

#define WM_SOCKET_TCP (WM_USER + 1)
#define WM_CONNECT_CLIENT (WM_USER + 3)
#define WM_CLIENT_TCP (WM_USER + 4)
#define WM_CLIENT_UDP (WM_USER + 5)
#define WM_DISCONNECT_CLIENT (WM_USER + 6)
#define WM_TCP_SERVER_LISTEN (WM_USER + 7)
#define WM_CONNECT_SERVER (WM_USER + 8)
#define WM_DISCONNECT_SERVER (WM_USER + 9)

#define DEFAULT_PORT 5000
#define DEFAULT_PAC_SIZE 128
#define DEFAULT_PAC_TIMES 100
#define STR_SIZE 512
#define BUF_SIZE 100000
#define PACKET_STR_MAX 128
#define IP_LENGTH 64

#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
	OVERLAPPED Overlapped;
	char Buffer[BUF_SIZE];
	WSABUF DataBuf;
	SOCKET Socket;
	DWORD BytesSEND;
	DWORD BytesRECV;
	DWORD PacketsRECV;
	DWORD SentBytesTotal;
	DWORD SentNumberPackets;
	DWORD SentPacketSize;
	DWORD IsFile;
	std::string FileName;
	std::vector<std::string> vecBuffer;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

enum PACKET_HEADER
{
	PH_INFO_SONG = 1,
	PH_INFO_CLIENT,

	PH_NUM_OF_SONGS,
	PH_NUM_OF_CLIENT,
	PH_SID,

	PH_REQ_SONG,

	SIZE_OF_HADER
};

typedef struct _CONTROL_MSG
{
	int header;
	union
	{
		int numOfSongs;
		int numOfClient;
		int SID;
	} msg;
} CONTROL_MSG, *LPCONTROL_MSG;

typedef struct _INFO_SONG
{
	int header;
	int SID;
	char title[PACKET_STR_MAX];
	char artist[PACKET_STR_MAX];
} INFO_SONG, *LPINFO_SONG;

typedef struct _INFO_CLIENT
{
	int header;
	char username[PACKET_STR_MAX];
	char ip[IP_LENGTH];
} INFO_CLIENT, *LPINFO_CLIENT;

typedef struct _SongData {
	int header;
	int SID;
	char title[PACKET_STR_MAX];
	char artist[PACKET_STR_MAX];
	char filename[FILENAME_MAX];
} SongData, *LPSongData;

extern HWND g_hWnd;
extern HWND g_hMainDlg;
extern HWND g_hResult;
extern BOOL g_bIsServer;
extern char g_IP[STR_SIZE];
extern unsigned int g_lTotalRecvBytes;
extern int g_port;
extern BOOL g_bIsFile;
extern int g_packetSize;
extern int g_packetTimes;
extern char g_filename[STR_SIZE];

inline void Display(const char* str)
{
	ListBox_InsertString(g_hResult, -1, str);
	ListBox_SetCurSel(g_hResult, ListBox_GetCount(g_hResult) - 1);
	SetFocus(g_hResult);
	//SendMessage(g_hResult, LB_GETCURSEL, 0, 0);
	//SendMessage(g_hResult, LB_ADDSTRING, 0, (LPARAM)str);
}

inline const std::wstring GetWC(const char *c)
{
	const size_t cSize = strlen(c) + 1;
	std::wstring wc(cSize, L'#');
	size_t outSize;
	mbstowcs_s(&outSize, &wc[0], cSize, c, cSize - 1);

	return wc;
}

void SaveSongFile(const std::string& fileName, std::vector<std::string>& data);
