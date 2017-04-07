#include "ClientTCP.h"
using namespace std;


/*-----------------------------------------------------------------------
--  SOURCE FILE:    client.cpp
--
--  PROGRAM:        COMP 4981 COMM Audio
--
--  FUNCTIONS:
--                  int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--							  		  LPSTR lspszCmdParam, int nCmdShow);
--                  LRESULT CALLBACK Idle(HWND, UINT, WPARAM, LPARAM);
--
--
--  DATE:           Mar 27, 2017
--
--  DESIGNER:       Aing Ragunathan
--
--
--  NOTES:
--					This program consists of a client that is capable of connecting
--					to a server and listen to its audio broadcast. And is able to 
--					request songs to play as well as transfer audio files entirely.
--					The client is also able to connect to other clients and 
--					create a voip chat session.
----------------------------------------------------------------------------*/





/*---------------------------------------------------------------------------------
--  FUNCTION:		WinMain
--
--  DATE:			Mar 20, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--
--  INTERFACE:		int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance,
--                                  	LPSTR lspszCmdParam, int nCmdShow)
--
--
--  RETURNS:		WM_QUITS message value or 0 if no messages sent
--
--  NOTES:
--					This function creates the main window, registers it, displays it
--					and sends messages.
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
--  FUNCTION:		Idle
--
--  DATE:			Mar 20, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:		LRESULT CALLBACK Idle(HWND hwnd, UINT Message,
--	                                        WPARAM wParam, LPARAM lParam)
--                           HWND hwnd: A handle to the window
--                           UINT Message: Recieved message
--                           WPARAM wParam: Additional message information
--                           LPARAM lParam: Additional message information
--
--  RETURNS:      
--
--  NOTES:
--					Idle state for managing connections, threads and UI
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
			//go to UDP listening
			break;
		case IDM_VOIP:
			hThreadArray[2] = CreateThread(NULL, 0, microphoneStart, NULL, 0, &dwThreadIdArray[2]); //start microphone

			//go to UDP lisenting
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
--  FUNCTION:		Connect
--
--  DATE:			Mar 20, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:  
--					TCHAR[16] ip = ip address of server
--					int port = port number of server
--
--  RETURNS:		void
--
--  NOTES:
--					Connects to the server for tcp commands only.
--					Uses default server ip or port isn't included.
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

	hThreadArray[1] = CreateThread(NULL, 0, recvCommand, NULL, 0, &dwThreadIdArray[1]);

	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		command
--
--  DATE:			Mar 29, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:
--
--  RETURNS:
--
--  NOTES:
--					Threaded function to receive messages from the server to update the
--					client and song list.	
--				
--					FYI might be replaced by idle
-----------------------------------------------------------------------------------*/
DWORD WINAPI command(LPVOID lpParam){
	//GUI COMMANDS GOES HERE
		//request to play a song
		//upload song to server
		//download song from server

	return 0;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		uploadFile
--
--  DATE:			Mar 27, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:		none
--
--  RETURNS:		void
--
--  NOTES:
--					Sends a contol message to the server before sending a file.
-----------------------------------------------------------------------------------*/
bool uploadFile() {
	HANDLE fileOutputHandle;	//Handle to the requested file to send
	TCHAR fileBuffer[PACKET_SIZE] = { 0 };	//buffer for file
	//TCHAR filename[MAX_PATH] = "McLaren.txt";
	TCHAR filename[MAX_PATH] = TEST_FILE;
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
--  FUNCTION:		downloadFile
--
--  DATE:			Mar 27, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:	
--
--  RETURNS:
--
--  NOTES:
--					Receives a file from the server. Number of packets must be specified.
-----------------------------------------------------------------------------------*/
bool downloadFile() {

	//differentiate file packet from command
	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		recvCommand
--
--  DATE:			Mar 27, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:
--
--  RETURNS:
--
--  NOTES:
--					Threaded function used to receive commands from the server. Waits for
--					control messages on a loop until the program ends. 
--					Command messages can update the client or songs list.
-----------------------------------------------------------------------------------*/
DWORD WINAPI recvCommand(LPVOID lpParam) {
	while (true) {
		recvServerMessage();	//update song and client lists
	}
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		Connect
--
--  DATE:			Mar 27, 2017
--
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:
--
--  RETURNS:
--
--  NOTES:
--					Receives control messages from the server and updates lists accordingly
--					or initiates downloading a file from the server.
-----------------------------------------------------------------------------------*/
bool recvServerMessage() {
	char messageBuffer[PACKET_SIZE];
	ControlMessage *controlMessage;
	ClientData *clientData;
	SongData *songData;
		
	//Get header from server
	if (recv(Socket, messageBuffer, sizeof(int), 0) == -1) {
		perror("recvServerMessage - Recv control message failed!");
		return false;
	}

	//manage server update or download response
	switch (atoi(messageBuffer))
	{
	case SONG_UPDATE:
		if (recv(Socket, messageBuffer, sizeof(SongData) - sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		songData = (SongData *)messageBuffer;	//extract song from buffer
		//songs.push_back(SongData());	//create a new song object in the vector
		SongData recvSongData;
		recvSongData.SID = songData->SID;	//copy song id over
		sprintf_s(recvSongData.artist, STR_MAX_SIZE, "%s", songData->artist);
		sprintf_s(recvSongData.title, STR_MAX_SIZE, "%s", songData->title);
		songs.push_back(recvSongData);
		break;
	case CLIENT_UPDATE:
		if (recv(Socket, messageBuffer, sizeof(ClientData) - sizeof(int), 0) == -1) {
			perror("recvServerMessage - Recv control message failed!");
			return false;
		}
		clientData = (ClientData *)messageBuffer;	//extract client from buffer
		//clients.push_back(ClientData());	//create a new client object in the vector
		ClientData recvClientData;
		sprintf_s(recvClientData.username, STR_MAX_SIZE, "%s", clientData->username);
		sprintf_s(recvClientData.ip, STR_MAX_SIZE, "%s", clientData->ip);
		break;
	case SONG_REQUEST:
		//get file from server and save to disk
		if (!downloadFile()) {
			return false;
		}
		break;
	}

	return true;
}

/*---------------------------------------------------------------------------------
--  FUNCTION:		requestSong
--
--  DATE:			April 3, 2017
--	
--  DESIGNER:		Aing Ragunathan
--
--  INTERFACE:	 
					int song - SID of the song requested
--
--  RETURNS:		none
--
--  NOTES:
--					Sends a song request to the server.
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