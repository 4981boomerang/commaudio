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

//Win32 Headers
#include <atlstr.h>  
#include <CommCtrl.h>

//LibZPlay Header
#include "../Headers/libzplay.h"

//Custom Headers
#include "../Headers/Main.h"
#include "../Headers/Network.h"
#include "../Headers/UI.h"

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
	UI ui(hDlg);
	Network networkManager(&ui);

	//connection thread
	HANDLE bgTCPThread = NULL, bgUDPThread = NULL;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		//temp UI stuff for now
		ui.setText(IDC_TRACKNAME, "Placeholder Song Title");
		ui.setText(IDC_ARTIST, "Placeholder Artist Title");
		ui.setText(IDC_ALBUM, "Placeholder Album Title");
		//---------------------------------------------------------------
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDM_FILE:
			break;

		case IDM_SETTINGS:
			break;

		case IDM_HELP:
			ui.showMessageBox("[PLACEHOLDER]", "Com Audio Help", MB_OK | MB_ICONINFORMATION);
			break;

		case IDM_ABOUT:
			ui.showMessageBox("Com Audio Developers:\n* Eva Yu\n* Jamie Lee\n* Aing Ragunathan\n* Michael Goll\n", "About Boomerang", MB_OK | MB_ICONQUESTION);
			break;

		case IDM_EXIT:
			PostQuitMessage(0);
			break;

			WSACleanup();
			break;

		case IDC_CONNECT:
			if (ui.checkUserInput()) {
				std::string dest = ui.getDestination();
				int port = ui.getPort();
				networkManager.setIP(dest);
				networkManager.setPort(port);
				networkManager.clientStart();

				networkManager.startTCPthread(); 
				networkManager.startUDPthread(); 
			}

			break;

		case IDC_DISCONNECT:
			//disconnect here, network cleanup
			ui.swapButtons(IDC_DISCONNECT, IDC_CONNECT);
			networkManager.clientStop(TRUE, TRUE);
			break;

		case IDC_CHOOSEFILE:
			ui.getFilePath();
			break;

		case IDC_UPLOAD:
			break;

		case IDC_PLAY:
			//change the "Play" button to "Pause"
			ui.swapButtons(IDC_PLAY, IDC_PAUSE);
			AudioPlayer::instance().play();
			break;

		case IDC_PAUSE:
			//change the "Pause" button to "Play"
			ui.swapButtons(IDC_PAUSE, IDC_PLAY);
			AudioPlayer::instance().pause();
			break;

		case IDC_NEXT:
			break;

		case IDC_PREV:
			break;

		case IDC_CALL:
			//spawn microphone thread here
			break;

		}
		break;
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

int __stdcall callBackFunc(void * instance, void * userData, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2) {
	const auto ap = AudioPlayer::instance().getPlayer();
	auto buffer = AudioPlayer::instance().pop();

	switch (message) {
	//player needs more data to play
	case libZPlay::MsgStreamNeedMoreData:
		//grabs data from the circular buffer and copies it to the player's internal buffer
		ap->PushDataToStream(buffer, BUFSIZE);
		break;
	}
	return 0;
}


