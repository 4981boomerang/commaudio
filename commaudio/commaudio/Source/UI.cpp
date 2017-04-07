/*------------------------------------------------------------------------------------------------
-- SOURCE FILE: UI.cpp
--
-- PROGRAM:     COMP4985_Assignment_2
--
-- FUNCTIONS:   void updateUI(HWND, bool)
--              void setText(HWND, int, char *)
--              void updateStatusText(HWND, char *)
--              void getUserInput(HWND)
--              void showMessageBox(HWND, char *, char *, int)
--              bool validateDest(char *)
--              char * getFileName(HWND)
--              void swapButtons(int, int)
--
-- DATE:        March 27, 2017
--
-- DESIGNER:    Michael Goll
--
-- PROGRAMMER:  Michael Goll
--
-- REVISIONS:   Changed procedural UI to a class | Michael Goll | April 3, 2017
--
-- NOTES:       Responsible for changing/updating any element on the user interface.
--------------------------------------------------------------------------------------------------*/

#include "../Headers/UI.h"

/* ----------------------------------------------------------------------------
-- FUNCTION:   updateUI
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void updateUI(HWND, bool)
--
-- PARAMETER:  bool option - the option to either disable or enable the user
--                           interface elements
--
-- RETURNS:    void
--
-- NOTES:      Updates all user interface elements to disabled or enabled
-----------------------------------------------------------------------------*/
void UI::updateUI(bool option) {
	
}

/* ----------------------------------------------------------------------------
-- FUNCTION:   setText
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  bool getCheck(HWND, int)
--
-- PARAMETER:  int item       - The user interface element to be grabbed
--             char * message - The message to set the text to.
--
-- RETURNS:    void
--
-- NOTES:      Sets the text of any given window element.
--
-----------------------------------------------------------------------------*/
void UI::setText(int item, char * message) {
	SetWindowText(GetDlgItem(hDlg, item), message);
}


/* ----------------------------------------------------------------------------
-- FUNCTION:   updateStatusText
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void updateStatusText(HWND, char *)
--
-- PARAMETER:  char * message - The message to update the text with.
--
-- RETURNS:    void
--
-- NOTES:      Updates the text above the progress bar.
-----------------------------------------------------------------------------*/
void UI::updateStatusText(char * message) {
}


/* ----------------------------------------------------------------------------
-- FUNCTION:   getUserInput
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void getUserInput(HWND)
--
-- PARAMETER:  none
--
-- RETURNS:    bool - Whether or not the user's input was valid.
--
-- NOTES:      Gets the user input from the various text fields
-----------------------------------------------------------------------------*/
bool UI::getUserInput() {
	char dest[32], port[32], pNum[32], pSize[32];
	int inputSize = 32;
	int portNum, packNum, packSize;

	GetDlgItemText(hDlg, IDC_IP, dest, inputSize);
	if (strcmp(dest, "") == 0) {
		showMessageBox("IP Address is empty, please specify an IP address", "IP Address Error", MB_OK | MB_ICONERROR);
		return false;
	}
	
	GetDlgItemText(hDlg, IDC_PORT, port, inputSize);
	if (strcmp(port, "") == 0) {
		showMessageBox("Port number is empty, please specify a port number", "Port Number Error", MB_OK | MB_ICONERROR);
		return false;
	}

	portNum = atoi(port);
	swapButtons(IDC_CONNECT, IDC_DISCONNECT);

	return true;
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   validateDest
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void setStatNum(const HWND, const int, const unsigned int)
--
-- PARAMETER:  char * dest - The destination specified by the user
--
-- RETURNS:    void
--
-- NOTES:      Validates that the destination starts with a character or digit.
--------------------------------------------------------------------------------------------*/
bool UI::validateDest(char *dest) {
	return (isalpha(dest[0]) || isdigit(dest[0]));
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   showMessageBox
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void showMessageBox(HWND, char *, char *, int)
--
-- PARAMETER:  char * message - The message to be displayed in the message box.
--             char * title   - The title of the message box
--             int iconOption - The icon to be displayed in the message box (ex. MB_OK)
--
-- RETURNS:    void
--
-- NOTES:      Spawns a message box with a specified message, title and icon.
--------------------------------------------------------------------------------------------*/
void UI::showMessageBox(char * message, char * title, int iconOption) {
	MessageBox(hDlg, message, title, MB_OK | iconOption);
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   getFilePath
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  int getFilePath(HWND)
--
-- PARAMETER:  none
--
-- RETURNS:    void
--
-- NOTES:      Opens a dialog and allows the user to select a file.
--------------------------------------------------------------------------------------------*/
int UI::getFilePath() {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);

	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object.
		HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			// Show the Open dialog box.
			hr = pFileOpen->Show(NULL);

			// Get the file name from the dialog box.
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);


					if (SUCCEEDED(hr))
					{
						SetWindowTextW(GetDlgItem(hDlg, IDC_FILEPATH), pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
	return 0;
}


/*--------------------------------------------------------------------------------------------
-- FUNCTION:   getFileName
--
-- DATE:       March 27, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  char * getFileName()
--
-- PARAMETER:  none
--
-- RETURNS:    void
--
-- NOTES:      Grabs the path from the textbox on the user interface.
--------------------------------------------------------------------------------------------*/
char * UI::getFileName() {
	char name[300];
	GetDlgItemText(hDlg, IDC_FILEPATH, name, 300);
	return name;
}

/*--------------------------------------------------------------------------------------------
-- FUNCTION:   swapButtons
--
-- DATE:       March 29, 2017
--
-- DESIGNER:   Michael Goll
--
-- PROGRAMMER: Michael Goll
--
-- INTERFACE:  void UI::swapButtons(int btnToHide, int btntoShow)
--
-- PARAMETER:  int btnToHide - User interface button to swap out/hide from view.
--             int btnToShow - User interface button to swap in/show.
--
-- REVISIONS:  Changed name from "updateConnectionButtons" to "swapButtons" to accomodate for
--             better general use cases. | Michael Goll | April 3, 2017
--
-- RETURNS:    void
--
-- NOTES:      Takes two specified user interface elements and "swaps" them in the view of
--             the user.
--------------------------------------------------------------------------------------------*/
void UI::swapButtons(int btnToHide, int btntoShow) {
	ShowWindow(GetDlgItem(hDlg, btnToHide), FALSE);
	ShowWindow(GetDlgItem(hDlg, btntoShow), TRUE);
}

void UI::changeAlbumPicture() {

}
