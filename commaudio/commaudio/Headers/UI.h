#ifndef UI_H
#define UI_H
/*------------------------------------------------------------------------------------------------
-- SOURCE FILE: UI.cpp
--
-- PROGRAM:     COMP4985_Assignment_2
--
-- FUNCTIONS:   void updateUI(bool)
--              void setText(int, char *)
--              void updateStatusText(char *)
--              void checkUserInput()
--              void showMessageBox(char *, char *, int)
--              bool validateDest(char *)
--              char * getFileName()
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
#define _WINSOCKAPI_

//Windows Headers
#include <Windows.h>
#include <windowsx.h>
#include <ShObjIdl.h>
#include <commctrl.h>

//C/C++ Headers
#include <string.h>
#include <string>
#include <sstream>

//Custom Headers
#include "../Source/resource.h"

class UI {
public:

	UI(HWND hwnd) : hDlg(hwnd), itemIndex(0) {}
	~UI() = default;

	/* ----------------------------------------------------------------------------
	-- FUNCTION:   updateUI
	--
	-- DATE:       March 27, 2017
	--
	-- DESIGNER:   Michael Goll
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  void updateUI(bool)
	--
	-- PARAMETER:  bool option - the option to either disable or enable the user
	--                           interface elements
	--
	-- RETURNS:    void
	--
	-- NOTES:      Updates all user interface elements to disabled or enabled
	-----------------------------------------------------------------------------*/
	void updateUI(bool);


	/* ----------------------------------------------------------------------------
	-- FUNCTION:   setText
	--
	-- DATE:       March 27, 2017
	--
	-- DESIGNER:   Michael Goll
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  bool getCheck(int)
	--
	-- PARAMETER:  int item       - The user interface element to be grabbed
	--             char * message - The message to set the text to.
	--
	-- RETURNS:    void
	--
	-- NOTES:      Sets the text of any given window element.
	--
	-----------------------------------------------------------------------------*/
	void setText(int, char *);


	/* ----------------------------------------------------------------------------
	-- FUNCTION:   updateStatusText
	--
	-- DATE:       March 27, 2017
	--
	-- DESIGNER:   Michael Goll
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  void updateStatusText(char *)
	--
	-- PARAMETER:  char * message - The message to update the text with.
	--
	-- RETURNS:    void
	--
	-- NOTES:      Updates the text above the progress bar.
	-----------------------------------------------------------------------------*/
	void updateStatusText(char *);


	/* ----------------------------------------------------------------------------
	-- FUNCTION:   checkUserInput
	--
	-- DATE:       March 27, 2017
	--
	-- DESIGNER:   Michael Goll
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  void checkUserInput()
	--
	-- PARAMETER:  none
	--
	-- RETURNS:    bool - Whether or not the user's input was valid.
	--
	-- NOTES:      Gets the user input from the various text fields
	-----------------------------------------------------------------------------*/
	bool checkUserInput();

	/*--------------------------------------------------------------------------------------------
	-- FUNCTION:   showMessageBox
	--
	-- DATE:       March 27, 2017
	--
	-- DESIGNER:   Michael Goll
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  void showMessageBox(char *, char *, int)
	--
	-- PARAMETER:  char * message - The message to be displayed in the message box.
	--             char * title   - The title of the message box
	--             int iconOption - The icon to be displayed in the message box (ex. MB_OK)
	--
	-- RETURNS:    void
	--
	-- NOTES:      Spawns a message box with a specified message, title and icon.
	--------------------------------------------------------------------------------------------*/
	void showMessageBox(char *, char *, int);

	/*--------------------------------------------------------------------------------------------
	-- FUNCTION:   validateDest
	--
	-- DATE:       March 27, 2017
	--
	-- DESIGNER:   Michael Goll
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  void setStatNum(const  const int, const unsigned int)
	--
	-- PARAMETER:  char * dest - The destination specified by the user
	--
	-- RETURNS:    boolean - Whether or not the destination specifed is valid.
	--
	-- NOTES:      Validates that the destination starts with a character or digit.
	--------------------------------------------------------------------------------------------*/
	bool validateDest(char *);

	/*--------------------------------------------------------------------------------------------
	-- FUNCTION:   getFilePath
	--
	-- DATE:       February 22, 2017
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
	int getFilePath();

	/*--------------------------------------------------------------------------------------------
	-- FUNCTION:   getFileName
	--
	-- DATE:       February 22, 2017
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
	char * getFileName();

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
	void swapButtons(int, int);

	/*--------------------------------------------------------------------------------------------
	-- FUNCTION:   getDestination
	--
	-- DATE:       April 7, 2017
	--
	-- DESIGNER:   Michael Goll
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  std::string UI::getDestination()
	--
	-- PARAMETER:  none
	--
	-- REVISIONS:
	--
	-- RETURNS:    std::string - The IP address specified by the user
	--
	-- NOTES:      Gets the IP address specified by the user from the user interface.
	--------------------------------------------------------------------------------------------*/
	std::string getDestination();

	/*--------------------------------------------------------------------------------------------
	-- FUNCTION:   getPort
	--
	-- DATE:       April 7, 2017
	--
	-- DESIGNER:   Michael Goll
	--
	-- PROGRAMMER: Michael Goll
	--
	-- INTERFACE:  int UI::getPort()
	--
	-- PARAMETER:  none
	--
	-- REVISIONS:
	--
	-- RETURNS:    int - The port number specified by the user.
	--
	-- NOTES:      Gets the port number specifed by the user.
	--------------------------------------------------------------------------------------------*/
	int getPort();

	void changeAlbumPicture();

	void addSingleListItem();

	private:
		HWND hDlg;
		int itemIndex;
};


#endif