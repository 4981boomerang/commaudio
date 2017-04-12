/*------------------------------------------------------------------------------------------------
-- SOURCE FILE: Main.cpp
--
-- PROGRAM:     COMP4985_Assignment_2
--
-- FUNCTIONS:   INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
--              int WinMain(GINSTANCE, HINSTANCE, LPSTR, int)
--
-- DATE:        March 27, 2017
--
-- DESIGNER:    Michael Goll
--
-- PROGRAMMER:  Michael Goll
--
-- NOTES:       Creates and displays the user interface.
--------------------------------------------------------------------------------------------------*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma warning (disable: 4996)
#pragma warning (disable: 4096)

#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "libzplay.lib")

//Custom Headers
#include "Main.h"

libZPlay::ZPlay * player = libZPlay::CreateZPlay();
int __stdcall callbackFunc(void * instance, void * userData, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);

extern commonResources common;
/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   DialogProc
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
--
-- PARAMETER:  HWND hDlg     - A handle to the dialog to spawn the message box.
--             UINT uMsg     - The Window message that is passed to the application
--             WPARAM wParam - The paramater that is passed to the window.
--             LPARAM lParam - Another parameter passed to the window.
--
-- RETURNS:    int
--
-- NOTES:      This function handles the messages that Windows sends to the window.
--
----------------------------------------------------------------------------------------------- */
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc = (HDC)wParam;
	LVITEM lvI;
	LVTILEVIEWINFO tvi = { 0 };
	HWND songList = GetDlgItem(hDlg, IDC_SONGLIST);
	common.player = player;

	//connection thread
	HANDLE bgTCPThread = NULL, bgUDPThread = NULL, uploadthread = NULL;

	switch (uMsg)
	{
	case WM_INITDIALOG:

		SetDlgItemText(hDlg, IDC_EDIT1, "Not Connected");

		SendMessage(songList, LVM_SETVIEW, (WPARAM)LV_VIEW_TILE, 0);

		tvi.cbSize = sizeof(LVTILEVIEWINFO);
		tvi.dwMask = LVTVIM_COLUMNS;
		tvi.cLines = 2; // 2 subitems
		SendMessage(songList, LVM_SETTILEINFO, 0, (LPARAM)&tvi);

		char temp[256];

		memset(&lvI, 0, sizeof(lvI));

		for (int i = 0; i < 5; ++i) {
			lvI.mask = LVIF_TEXT | LVIF_IMAGE; //text style
			lvI.cchTextMax = 256; //max size of temp
			lvI.iItem = i; //choose which item
			lvI.iSubItem = 0; //put in first column
			lvI.pszText = "ITEM 0"; //display text
			SendMessage(songList, LVM_INSERTITEM, 0, (LPARAM)&lvI); //send this info to the list control

			lvI.iSubItem = i;
			lvI.pszText = "[TEMPLATE SONG LIST ENTRY]";
			SendMessage(songList, LVM_SETITEM, 0, (LPARAM)&lvI);
		}
		
		//temp UI stuff for now
		setText(hDlg, IDC_TRACKNAME, "Placeholder Song Title");
		setText(hDlg, IDC_ARTIST, "Placeholder Artist Title");
		setText(hDlg, IDC_ALBUM, "Placeholder Album Title");
		//---------------------------------------------------------------
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDM_FILE:
			break;

		case IDM_SETTINGS:
			break;

		case IDM_HELP:
			showMessageBox(hDlg, "Enter an IP address into the \"IP Address\" box and the port number and click \"Connect.\"", "Com Audio Help", MB_OK | MB_ICONINFORMATION);
			break;

		case IDM_ABOUT:
			showMessageBox(hDlg, "Com Audio Developers:\n* Eva Yu\n* Jamie Lee\n* Aing Ragunathan\n* Michael Goll\n", "About Boomerang", MB_OK | MB_ICONQUESTION);
			break;

		case IDM_EXIT:
			PostQuitMessage(0);
			break;

			WSACleanup();
			break;

		case IDC_CONNECT:
			if (checkUserInput(hDlg)) {
				clientStart(hDlg);
				//TODO: initialize the audio stream
				bgTCPThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startTCP, (LPVOID)hDlg, 0, 0);
				bgUDPThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startUDP, (LPVOID)hDlg, 0, 0);
				swapButtons(hDlg, IDC_CONNECT, IDC_DISCONNECT);
			}

			break;

		case IDC_DISCONNECT:
			//disconnect here, network cleanup
			clientStop(TRUE, TRUE);
			WSACleanup();

			if (bgTCPThread != NULL)
				TerminateThread(bgTCPThread, 0);

			if (bgUDPThread != NULL)
				TerminateThread(bgUDPThread, 0);

			swapButtons(hDlg, IDC_DISCONNECT, IDC_CONNECT);
			SetDlgItemText(hDlg, IDC_EDIT1, "Not Connected");
			break;

		case IDC_CHOOSEFILE:
			getFilePath(hDlg, TRUE);
			break;

		case IDC_CHOOSEFILE2:
			getFilePath(hDlg, FALSE);
			break;

		case IDC_UPLOAD:
			uploadthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)uploadFile, (LPVOID)hDlg, 0, 0);
			break;

		case IDC_DOWNLOAD:
			break;

		case IDC_PLAY:
			//change the "Play" button to "Pause"
			swapButtons(hDlg, IDC_PLAY, IDC_PAUSE);
			play(hDlg, player);
			break;

		case IDC_PAUSE:
			//change the "Pause" button to "Play"
			swapButtons(hDlg, IDC_PAUSE, IDC_PLAY);
			pause(hDlg, player);
			break;

		case IDC_NEXT:
			break;

		case IDC_PREV:
			break;

		case IDC_CALL:
			//spawn microphone thread here
			uploadthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)uploadFile, (LPVOID)hDlg, 0, 0);
			break;
		}
		break;


	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_CLOSE:
		//terminate running thread if it exists
		if (bgTCPThread != NULL)
			TerminateThread(bgTCPThread, 0);

		if (bgUDPThread != NULL)
			TerminateThread(bgUDPThread, 0);
		
		//cleanup network stuff just in case
		WSACleanup();
		DestroyWindow(hDlg);
		break;
	}
	return FALSE;
}

/*------------------------------------------------------------------------------------------------------------
-- FUNCTION:   WinMain
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
--
-- PARAMETER:  HINSTANCE hInst         - A handle to the instance, used by the OS
--             HINSTANCE hprevInstance - No longer used by the OS
--             LPSTR lspszCmdParam     - Command line arguments as a Unicode string
--             int nCmdShow            - Flag that states whether the main window will be minimized, ect
--
-- RETURNS:    int
--
-- NOTES:      Main execution entrance point for the application.
--             Creates the main window that will contain the program.
------------------------------------------------------------------------------------------------------------*/

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hprevInstance, LPSTR lspszCmdParam, int nCmdShow)
{
	HWND hDlg;
	MSG msg;
	BOOL ret;
	HMENU hMenu;

	INITCOMMONCONTROLSEX initCtrls;
	initCtrls.dwSize = sizeof(initCtrls);
	initCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&initCtrls);

	hDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN), 0, DialogProc, 0);
	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU3));
	SetMenu(hDlg, hMenu);
	ShowWindow(hDlg, nCmdShow);
	UpdateWindow(hDlg);

	common.hDlg = hDlg;
	
	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1)
			return -1;

		if (!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}