// BoomerangCommAudio.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <Winsock2.h>
#include <Commdlg.h>
#include "Common.h"
#include "BoomerangCommAudio.h"
#include "Server.h"


#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND hRadioServer;
HWND hRadioClient;
HWND hRadioIP;
HWND hRadioHost;
HWND hIP;
HWND hHost;
HWND hPort;
HWND hNumPacket;
HWND hSizePacket;
HWND hOpenFile;
HWND hSendFile;
HWND hSubmit;
HWND hConnect;
HWND hDisConnect;

SOCKET serverSock;
SOCKET clientSock;

Server tcpServer;

// Forward declarations of functions included in this code module:
BOOL                InitInstance(HINSTANCE, int);
INT_PTR CALLBACK	MainDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void GetDlgItems();
void SetEnableDlgItems(BOOL bIsServer);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.


	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	g_hMainDlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_MAIN_DLG), 0, MainDlg, 0);

	if (!g_hMainDlg)
	{
		return FALSE;
	}

	ShowWindow(g_hMainDlg, SW_SHOW);
	ShowWindow(g_hMainDlg, nCmdShow);
	UpdateWindow(g_hMainDlg);

	CheckDlgButton(g_hMainDlg, IDC_RADIO_SERVER, TRUE);
	CheckDlgButton(g_hMainDlg, IDC_RADIO_IP, TRUE);
	CheckDlgButton(g_hMainDlg, IDC_RADIO_TCP, TRUE);

	GetDlgItems();
	SetEnableDlgItems(TRUE);

	char temp[STR_SIZE] = "";
	sprintf_s(temp, STR_SIZE, "%d", g_port);
	SetDlgItemText(g_hMainDlg, IDC_EDIT_PORT, temp);
	sprintf_s(temp, STR_SIZE, "%d", g_packetSize);
	SetDlgItemText(g_hMainDlg, IDC_EDIT_SIZE_PAC, temp);
	sprintf_s(temp, STR_SIZE, "%d", g_packetTimes);
	SetDlgItemText(g_hMainDlg, IDC_EDIT_NUM_PAC, temp);

	return TRUE;
}

void GetDlgItems()
{
	hRadioServer = GetDlgItem(g_hMainDlg, IDC_RADIO_SERVER);
	hRadioClient = GetDlgItem(g_hMainDlg, IDC_RADIO_CLIENT);
	hRadioIP = GetDlgItem(g_hMainDlg, IDC_RADIO_IP);
	hRadioHost = GetDlgItem(g_hMainDlg, IDC_RADIO_HOST);
	hIP = GetDlgItem(g_hMainDlg, IDC_IPADDRESS);
	hHost = GetDlgItem(g_hMainDlg, IDC_EDIT_HOST);
	hPort = GetDlgItem(g_hMainDlg, IDC_EDIT_PORT);
	hNumPacket = GetDlgItem(g_hMainDlg, IDC_EDIT_NUM_PAC);
	hSizePacket = GetDlgItem(g_hMainDlg, IDC_EDIT_SIZE_PAC);
	hOpenFile = GetDlgItem(g_hMainDlg, IDC_BUTTON_OPEN_FILE);
	hSendFile = GetDlgItem(g_hMainDlg, IDC_BUTTON_SEND_FILE);
	hSubmit = GetDlgItem(g_hMainDlg, IDC_BUTTON_SUBMIT);
	g_hResult = GetDlgItem(g_hMainDlg, IDC_LIST_DISPLAY);
	hConnect = GetDlgItem(g_hMainDlg, IDC_CONNECT);
	hDisConnect = GetDlgItem(g_hMainDlg, IDC_DISCONNECT);
}

INT_PTR CALLBACK MainDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char buffer[STR_SIZE] = "";
	DWORD len;

	char strPort[STR_SIZE] = "";
	char strHost[STR_SIZE] = "";
	char strPacketSize[STR_SIZE] = "";
	char strPacketTimes[STR_SIZE] = "";
	g_bIsServer = IsDlgButtonChecked(g_hMainDlg, IDC_RADIO_SERVER) == BST_CHECKED;
	GetDlgItemTextA(g_hMainDlg, IDC_EDIT_PORT, strPort, STR_SIZE);
	len = GetWindowTextLengthA(GetDlgItem(g_hMainDlg, IDC_IPADDRESS));
	GetDlgItemTextA(g_hMainDlg, IDC_IPADDRESS, g_IP, len + 1);
	GetDlgItemTextA(g_hMainDlg, IDC_EDIT_HOST, strHost, STR_SIZE);
	GetDlgItemTextA(g_hMainDlg, IDC_EDIT_SIZE_PAC, strPacketSize, STR_SIZE);
	GetDlgItemTextA(g_hMainDlg, IDC_EDIT_NUM_PAC, strPacketTimes, STR_SIZE);
	if (strcmp(strPort, "") != 0)
	{
		g_port = atoi(strPort);
	}
	if (strcmp(strPacketSize, "") != 0)
	{
		g_packetSize = atoi(strPacketSize);
	}
	if (strcmp(strPacketTimes, "") != 0)
	{
		g_packetTimes = atoi(strPacketTimes);
	}

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_CLIENT_TCP:
	case WM_CLIENT_UDP:
		//ClientProc(hDlg, message, wParam, lParam, clientSock);
		break;

	case WM_SOCKET_TCP:
	case WM_TCP_SERVER_LISTEN:
		//ServerProc(hDlg, message, wParam, lParam, serverSock);
		break;

	case WM_COMMAND:
		/*if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
		EndDialog(hDlg, LOWORD(wParam));
		SendMessage(GetParent(hDlg), WM_DESTROY, 0, 0);
		//PostQuitMessage(0);
		return (INT_PTR)TRUE;
		}*/

		char strPort[STR_SIZE] = "";
		char strHost[STR_SIZE] = "";
		char strPacketSize[STR_SIZE] = "";
		char strPacketTimes[STR_SIZE] = "";

		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDC_RADIO_SERVER:
		case IDC_RADIO_CLIENT:
		case IDC_RADIO_TCP:
		case IDC_RADIO_UDP:
		case IDC_RADIO_IP:
		case IDC_RADIO_HOST:
			g_bIsServer = IsDlgButtonChecked(g_hMainDlg, IDC_RADIO_SERVER) == BST_CHECKED;
			SetEnableDlgItems(g_bIsServer);
			break;

		case IDC_CONNECT:
			if (g_bIsServer)
			{
				//RunServer(serverSock);
			}
			else
			{
				//ConnectClient(clientSock);
			}
			break;

		case IDC_BUTTON_OPEN_FILE:
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = g_hWnd;
			ofn.lpstrFilter = "Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
			ofn.lpstrFile = buffer;
			ofn.nMaxFile = STR_SIZE;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ALLOWMULTISELECT;
			ofn.lpstrDefExt = "txt";

			if (GetOpenFileName(&ofn))
			{
				SetWindowText(GetDlgItem(g_hMainDlg, IDC_EDIT_FILE), buffer);
			}
			break;

		case IDC_DISCONNECT:
			if (g_bIsServer)
			{
				PostMessage(g_hMainDlg, WM_TCP_SERVER_LISTEN, NULL, FD_CLOSE);
			}
			else
			{
				PostMessage(g_hMainDlg, WM_CLIENT_TCP, NULL, FD_CLOSE);
			}
			break;

		case IDC_BUTTON_SEND_FILE:
			len = GetWindowTextLengthA(GetDlgItem(g_hMainDlg, IDC_EDIT_FILE));
			GetWindowTextA(GetDlgItem(g_hMainDlg, IDC_EDIT_FILE), g_filename, len + 1);
			//sendFile(clientSock);
			break;

		case IDC_BUTTON_SUBMIT:

			if (g_bIsServer)
			{
				//RunServer(serverSock);
				tcpServer.RunServer(serverSock);
			}
			else
			{
				//SendPacket(clientSock);
			}
			break;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hDlg, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hDlg, &ps);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hDlg, message, wParam, lParam);
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void SetEnableDlgItems(BOOL bIsServer)
{
	SetWindowText(hSubmit, bIsServer == TRUE ? "Start Server" : "Send Packet");
	EnableWindow(hIP, !bIsServer);
	EnableWindow(hRadioIP, !bIsServer);
	EnableWindow(hRadioHost, !bIsServer);
	EnableWindow(hHost, !bIsServer);
	EnableWindow(hNumPacket, !bIsServer);
	EnableWindow(hSizePacket, !bIsServer);
	EnableWindow(hOpenFile, !bIsServer);
	EnableWindow(hSendFile, !bIsServer);
	EnableWindow(hConnect, !bIsServer);
	EnableWindow(hDisConnect, !bIsServer);

	if (!bIsServer)
	{
		BOOL bHost = IsDlgButtonChecked(g_hMainDlg, IDC_RADIO_HOST) == BST_CHECKED;
		if (bHost)
		{
			EnableWindow(hIP, FALSE);
			EnableWindow(hPort, FALSE);
			EnableWindow(hHost, TRUE);
		}
		else
		{
			EnableWindow(hIP, TRUE);
			EnableWindow(hPort, TRUE);
			EnableWindow(hHost, FALSE);
		}
	}
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
