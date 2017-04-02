/*-----------------------------------------------------------------------
--  SOURCE FILE:   client.cpp
--
--  PROGRAM:       COMP 4981 COMM Audio
--
--  FUNCTIONS:
--                 int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--							  		  LPSTR lspszCmdParam, int nCmdShow);
--                 LRESULT CALLBACK Idle(HWND, UINT, WPARAM, LPARAM);
--
--
--  DATE:          Mar 27, 2017
--
--  DESIGNER:      Aing Ragunathan
--
--
--  NOTES:
--				   	
----------------------------------------------------------------------------*/

#define STRICT
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _UNICODE

#define DEFAULT_PORT	5555	
#define WM_SOCKET		104
#define DATA_BUFSIZE	1024
#define MAX_THREADS		3


#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <streambuf>
#include "MenuSettings.h"
#include <tchar.h>
#include <strsafe.h>
#include <vector>


typedef struct {
	int header;
	union { 
		int numOfSongs; 
		int numOfClients;
		int SID;
	};
} ControlMessage;

typedef struct {
	char* username;;
	char* ip;	//used for inter-client communication
} ClientData;

typedef struct {
	int SID;
	char* title;
	char* artist;
} SongData;

using namespace std;

LRESULT CALLBACK Idle(HWND, UINT, WPARAM, LPARAM);
bool connect();
DWORD WINAPI command(LPVOID lpParam);
bool uploadFile();
bool downloadFile();

static TCHAR Name[] = TEXT("COMM Audio Client");	//Name of main window
HWND hwnd;											//Handle to main window
HINSTANCE mainInst;									//Instance of main window//at this application
SOCKET Socket = NULL;
HANDLE  hThreadArray[MAX_THREADS];
DWORD dwThreadIdArray[MAX_THREADS];
vector<SongData> songs;
vector<ClientData> clients;



/*---------------------------------------------------------------------------------
--  FUNCTION:      WinMain
--
--  DATE:          Feb 3, 2017
--
--  DESIGNER:      Aing Ragunathan
--
--
--  INTERFACE:     int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--                                  	LPSTR lspszCmdParam, int nCmdShow)
--
--
--  RETURNS:       WM_QUITS message value or 0 if no messages sent
--
--  NOTES:
--		This function creates the main window, registers it, displays it
--		and sends messages.
-----------------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
	LPSTR lspszCmdParam, int nCmdShow)
{
	MSG Msg;
	WNDCLASSEX Wcl;
	mainInst = hInst;

	Wcl.cbSize = sizeof(WNDCLASSEX);
	Wcl.style = CS_HREDRAW | CS_VREDRAW;
	Wcl.hIcon = LoadIcon(NULL, IDI_APPLICATION); // large icon 
	Wcl.hIconSm = NULL; // use small version of large icon
	Wcl.hCursor = LoadCursor(NULL, IDC_ARROW);  // cursor style

	Wcl.lpfnWndProc = Idle;
	Wcl.hInstance = hInst;
	Wcl.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //white background
	Wcl.lpszClassName = Name;

	Wcl.lpszMenuName = TEXT("MYMENU"); // The menu class
	Wcl.cbClsExtra = 0;      // no extra memory needed
	Wcl.cbWndExtra = 0;

	// Register the class
	if (!RegisterClassEx(&Wcl))
		return 0;

	hwnd = CreateWindow(Name, Name, WS_OVERLAPPEDWINDOW, 10, 10,
		475, 700, NULL, NULL, hInst, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, "Main window failed to open, program shutting down", "ERROR", MB_OK);
		return 0;
	}

	// Open up a Winsock v2.2 session
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);

	//initialize LibZPlay library

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0))
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:     Idle
--
--  DATE:         Mar 27, 2017
--
--  DESIGNER:     Aing Ragunathan
--
--  INTERFACE:    LRESULT CALLBACK Idle(HWND hwnd, UINT Message,
--	                                        WPARAM wParam, LPARAM lParam)
--                           HWND hwnd: A handle to the window
--                           UINT Message: Recieved message
--                           WPARAM wParam: Additional message information
--                           LPARAM lParam: Additional message information
--
--  RETURNS:      
--
--  NOTES:
--		
-----------------------------------------------------------------------------------*/
LRESULT CALLBACK Idle(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))  //Parsing the menu selections
		{
		case IDM_CONNECT:
			if (connect()) {
				hThreadArray[0] = CreateThread(NULL, 0, command, NULL, 0, &dwThreadIdArray[0]);   
			}
			break;
		case IDM_DOWNLOAD_FILE:
			downloadFile();
			break;
		case IDM_UPLOAD_PACKET:
			uploadFile();
			break;
		case IDM_MUSIC:
			
			break;
		case IDM_VOIP:

			break;
		}
		break;
		//END OF MENU SWITCH

	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		closesocket(Socket);
		WSACleanup();
		return 0;
	}
	//END OF MESSAGE SWITCH

	return DefWindowProc(hwnd, Message, wParam, lParam);
}

/*---------------------------------------------------------------------------------
--  FUNCTION:     Connect
--
--  DATE:         Mar 27, 2017
--
--  DESIGNER:     Aing Ragunathan
--
--  INTERFACE:    
--
--  RETURNS:      
--
--  NOTES:
--				Connects to the server for commands only
-----------------------------------------------------------------------------------*/
bool connect() {
	int Ret;					//check results of functions
	WSADATA WsaDat;
	struct hostent *host;
	TCHAR ip[16] = "127.0.0.1";	//get from UI
	int port = 5555;			//get from UI
	char messageBuffer[32];
	ControlMessage *controlMessage;
	SongData* songData;
	ClientData* clientData;

	//Resolve IP address
	if ((host = gethostbyname(ip)) == NULL)
	{
		MessageBox(hwnd,
			"Unable to resolve host name",
			"Error: GetHostByName",
			MB_ICONERROR);
		SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		return false;
	}

	//Create TCP socket
	if ((Socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		MessageBox(hwnd,
			"socket creation failed!",
			"Error: Socket",
			MB_ICONERROR);
		SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		return false;
	}

	// Set up our socket address structure
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(port);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

	//WRAP FROM HERE (FOLLOW DESIGN)

	// Connecting to the server
	if (connect(Socket, (struct sockaddr *)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
	{
		MessageBox(hwnd,
			"Failed to connect",
			"Error: Connect",
			MB_ICONERROR);
		return false;
	}

	//WRAP FROM HERE (UPDATE DESIGN, use substates)

	//block until Control message is received from the server for songs
	recv(Socket, messageBuffer, sizeof(ControlMessage), 0);
	controlMessage = (ControlMessage *)messageBuffer;

	//recv all the songs from the server
	for (int i = 0; i < controlMessage->numOfSongs; i++) {
		recv(Socket, messageBuffer, sizeof(SongData), 0);	//receive the next song object
		songData = (SongData *)messageBuffer;	//extract song from buffer
		songs.push_back(SongData());	//create a new song object in the vector
		songs[i].SID = songData->SID;	//copy song id over
		songs[i].artist = songData->artist;	//copy artist over
		songs[i].title = songData->title;	//copy title over
	}

	//block until Control message is received from the server for clients
	recv(Socket, messageBuffer, sizeof(ControlMessage), 0);
	controlMessage = (ControlMessage *)messageBuffer;

	//recv all the songs from the server
	for (int i = 0; i < controlMessage->numOfClients; i++) {
		recv(Socket, messageBuffer, sizeof(ClientData), 0);	//receive the next client info object
		clientData = (ClientData *)messageBuffer;	//extract client from buffer
		clients.push_back(ClientData());	//create a new client object in the vector
		clients[i].username = clientData->username;	//copy username over
		clients[i].ip = clientData->ip;	//copy ip over
	}


	return true;
}

DWORD WINAPI command(LPVOID lpParam){
	//GUI COMMANDS GOES HERE

	return 0;
}

bool uploadFile() {
	HANDLE fileOutputHandle;	//Handle to the requested file to send
	TCHAR fileBuffer[1024] = { 0 };	//buffer for file
	//TCHAR filename[MAX_PATH] = "McLaren.txt";
	TCHAR filename[MAX_PATH] = "hello.txt";
	OVERLAPPED ol = { 0 };
	DWORD bytesRead;
	string temp;


	fileOutputHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (fileOutputHandle == INVALID_HANDLE_VALUE) {
		MessageBox(hwnd,
			"Can not open file",
			"ReadFile: Failed!",
			MB_ICONERROR);
		return false;
	}
	else {

		

		do {
			if (!ReadFile(fileOutputHandle, fileBuffer, 1024 - 1, &bytesRead, &ol)) {
				printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
				MessageBox(hwnd,
					"Can not read from file",
					"ReadFile: Failed!",
					MB_ICONERROR);
				CloseHandle(fileOutputHandle);
				return false;
			}

			//append a null character to cut off the string if the entire buffer isn't used
			if (strlen(fileBuffer) < 1023) {
				fileBuffer[bytesRead] = '\0';
			}

			//send(Socket, fileBuffer, strlen(fileBuffer), 0); //send the buffer to the server		
			send(Socket, fileBuffer, bytesRead, 0); //send the buffer to the server		
			ol.Offset += bytesRead;	//move the reading window 
			temp = fileBuffer;	//reset the buffer
			temp.resize(1024);
		} while (strlen(fileBuffer) >= 1023);
	}


	return true;
}

bool downloadFile() {
	return true;
}