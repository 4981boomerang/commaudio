#ifndef MAIN_H
#define MAIN_H

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: Main.cpp
--
-- PROGRAM:     COMP4985_Assignment_2
--
-- FUNCTIONS:   INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
--              int WinMain(GINSTANCE, HINSTANCE, LPSTR, int)
--              void updateUI(HWND, bool)
--
-- DATE:        February 2, 2017
--
-- DESIGNER:    Michael Goll
--
-- PROGRAMMER:  Michael Goll
--
-- NOTES:       Creates and displays the user interface.
--              Responsible for resetting the user interface.
----------------------------------------------------------------------------------------------------------------------*/

#define WM_SOCKET (WM_USER + 0)

//Windows Headers
#include <winsock2.h>
#include <Windows.h>

//C/C++ Headers
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <thread>

//Custom Headers
#include "resource.h"
#include "UI.h"
#include "libzplay.h"
#include "Network.h"


#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int __stdcall callBackFunc(void * instance, void * userData, libZPlay::TCallbackMessage message, unsigned int param1, unsigned int param2);

/* ----------------------------------------------------------------------------------------------
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
-- RETURNS:    int
--
-- NOTES:      This function handles the messages that Windows sends.
--
------------------------------------------------------------------------------------------------ */
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);



/* ----------------------------------------------------------------------------
-- FUNCTION:   WinMain
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  int WinMain(GINSTANCE, HINSTANCE, LPSTR, int)
--
-- RETURNS:    int
--
-- NOTES:      Creating the main window that will contain the program.
--
-----------------------------------------------------------------------------*/
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

#endif