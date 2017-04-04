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
#define PACKET_SIZE		1024
#define MAX_THREADS		3
#define DEFAULT_IP		"127.0.0.1"


#include <winsock2.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <streambuf>
#include "MenuSettings.h"
#include <tchar.h>
#include <strsafe.h>
#include <vector>

#define SONG_LIST	 1
#define CLIENT_LIST  2
#define SONG_REQUEST 3


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
	char* ip;	
} ClientData;

typedef struct {
	int SID;
	char* title;
	char* artist;
} SongData;

using namespace std;

LRESULT CALLBACK Idle(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI command(LPVOID lpParam);
DWORD WINAPI recvCommand(LPVOID lpParam);
DWORD WINAPI connect(LPVOID lpParam);
bool uploadFile();
bool downloadFile();
bool recvServerMessage();

static TCHAR Name[] = TEXT("COMM Audio Client");	
HWND hwnd;											
HINSTANCE mainInst;									
SOCKET Socket = NULL;
HANDLE  hThreadArray[MAX_THREADS];
DWORD dwThreadIdArray[MAX_THREADS];
vector<SongData> songs;
vector<ClientData> clients;



/*---------------------------------------------------------------------------------
--  FUNCTION:      WinMain
--
--  DATE:          Mar 20, 2017
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
--  DATE:         Mar 20, 2017
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
--				Idle state for managing connections, threads and UI
-----------------------------------------------------------------------------------*/
LRESULT CALLBACK Idle(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))  //Parsing the menu selections
		{
		case IDM_CONNECT:
			hThreadArray[0] = CreateThread(NULL, 0, connect, NULL, 0, &dwThreadIdArray[0]);
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
--  FUNCTION:   Connect
--
--  DATE:       Mar 20, 2017
--
--  DESIGNER:   ing Ragunathan
--
--  INTERFACE:  
--				TCHAR[16] ip = ip address of server
--				int port = port number of server
--
--  RETURNS:    void
--
--  NOTES:
--				Connects to the server for tcp commands only.
--				Uses default server ip or port isn't included.
-----------------------------------------------------------------------------------*/
DWORD WINAPI connect(LPVOID lpParam){
	struct hostent *host;
	TCHAR ip[16] = DEFAULT_IP;
	int port = DEFAULT_PORT;

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

	// Connecting to the server
	if (connect(Socket, (struct sockaddr *)&SockAddr, sizeof(SockAddr)) == SOCKET_ERROR)
	{
		MessageBox(hwnd,
			"Failed to connect",
			"Error: Connect",
			MB_ICONERROR);
		return false;
	}

	//update song list
	if (!recvServerMessage()) {
		perror("connect - song list update failed!");
		return false;
	}

	//update client list
	if (!recvServerMessage()) {
		perror("connect - client list update failed!");
		return false;
	}

	hThreadArray[1] = CreateThread(NULL, 0, recvCommand, NULL, 0, &dwThreadIdArray[0]);

	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:     command
--
--  DATE:         Mar 29, 2017
--
--  DESIGNER:     Aing Ragunathan
--
--  INTERFACE:
--
--  RETURNS:
--
--  NOTES:
--				Threaded function to receive messages from the server to update the
--				client and song list.	
--				
--				FYI might be replaced by idle
-----------------------------------------------------------------------------------*/
DWORD WINAPI command(LPVOID lpParam){
	//GUI COMMANDS GOES HERE
		//request to play a song
		//upload song to server
		//download song from server

	return 0;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:	uploadFile
--
--  DATE:		Mar 27, 2017
--
--  DESIGNER:   Aing Ragunathan
--
--  INTERFACE:	none
--
--  RETURNS:	void
--
--  NOTES:
--				Sends a contol message to the server before sending a file.
-----------------------------------------------------------------------------------*/
bool uploadFile() {
	HANDLE fileOutputHandle;	//Handle to the requested file to send
	TCHAR fileBuffer[PACKET_SIZE] = { 0 };	//buffer for file
	//TCHAR filename[MAX_PATH] = "McLaren.txt";
	TCHAR filename[MAX_PATH] = "hello.txt";
	OVERLAPPED ol = { 0 };
	DWORD bytesRead;
	string temp;


	fileOutputHandle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	//validate file exists and send
	if (fileOutputHandle == INVALID_HANDLE_VALUE) {
		MessageBox(hwnd,
			"Can not open file",
			"ReadFile: Failed!",
			MB_ICONERROR);
		return false;
	}
	else {
		do {
			if (!ReadFile(fileOutputHandle, fileBuffer, PACKET_SIZE - 1, &bytesRead, &ol)) {
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
			temp.resize(PACKET_SIZE);
		} while (strlen(fileBuffer) >= PACKET_SIZE-1);
	}


	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:   downloadFile
--
--  DATE:       Mar 27, 2017
--
--  DESIGNER:   Aing Ragunathan
--
--  INTERFACE:	
--
--  RETURNS:
--
--  NOTES:
--				Receives a file from the server. Number of packets must be specified.
-----------------------------------------------------------------------------------*/
bool downloadFile() {

	//differentiate file packet from command
	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:     recvCommand
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
--				Threaded function used to receive commands from the server. Waits for
--				control messages on a loop until the program ends. 
--				Command messages can update the client or songs list.
-----------------------------------------------------------------------------------*/
DWORD WINAPI recvCommand(LPVOID lpParam) {
	while (true) {
		recvServerMessage();	//update song and client lists
	}
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
--				Receives control messages from the server and updates lists accordingly
--				or initiates downloading a file from the server.
-----------------------------------------------------------------------------------*/
bool recvServerMessage() {
	char messageBuffer[PACKET_SIZE];
	ControlMessage *controlMessage;
	ClientData *clientData;
	SongData *songData;
		
	//Get control message from server
	if (recv(Socket, messageBuffer, sizeof(ControlMessage), 0) == -1) {
		perror("recvServerMessage - Recv control message failed!");
		return false;
	}
	controlMessage = (ControlMessage *)messageBuffer;

	//manage server update or download response
	switch (controlMessage->header)
	{
	case SONG_LIST:
		//recv all the songs from the server			
		for (int i = 0; i < controlMessage->numOfSongs; i++) {
			if (recv(Socket, messageBuffer, sizeof(SongData), 0) == -1) {
				perror("recvServerMessage - Recv song data failed!");
				return false;
			}
			songData = (SongData *)messageBuffer;	//extract song from buffer
			songs.push_back(SongData());	//create a new song object in the vector
			songs[i].SID = songData->SID;	//copy song id over
			songs[i].artist = songData->artist;	//copy artist over
			songs[i].title = songData->title;	//copy title over
		}
		break;
	case CLIENT_LIST:
		//recv all the clients names from the server
		for (int i = 0; i < controlMessage->numOfClients; i++) {
			if (recv(Socket, messageBuffer, sizeof(ClientData), 0) == -1) {
				perror("recvServerMessage - Recv client data failed!");
				return false;
			}
			clientData = (ClientData *)messageBuffer;	//extract client from buffer
			clients.push_back(ClientData());	//create a new client object in the vector
			clients[i].username = clientData->username;	//copy username over
			clients[i].ip = clientData->ip;	//copy ip over
		}
		break;
	case SONG_REQUEST:
		//get file from server and save to disk
		if (downloadFile() == -1) {
			return false;
		}
		break;
	}

	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:   requestSong
--
--  DATE:       April 3, 2017
--
--  DESIGNER:   Aing Ragunathan
--
--  INTERFACE:	 
				int song - SID of the song requested
--
--  RETURNS:	none
--
--  NOTES:
--				Sends a song request to the server.
-----------------------------------------------------------------------------------*/
bool requestSong(int song) {
	ControlMessage *controlMessage;
	char *messageBuffer;
	
	//setup control message
	controlMessage = new ControlMessage();
	controlMessage->header = SONG_REQUEST;
	controlMessage->SID = song;
	messageBuffer = (char *)controlMessage;

	//send message to server
	if (send(Socket, messageBuffer, strlen(messageBuffer), 0) == -1)
		return false;

	return true;
}