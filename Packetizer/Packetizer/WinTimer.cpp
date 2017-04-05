/*------------------------------------------------------------------------------
-- SOURCE FILE: WinTimer.cpp - Timer Class
--
-- PROGRAM: COMP4985 A4
--
-- FUNCTIONS:
-- 
--
-- DATE: APR. 05, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/05 - Created Class 

-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- Timer class for window timer handling / wrappers
------------------------------------------------------------------------------*/
#include <iostream>
#include <iomanip>
#include "WinTimer.h"

using std::cerr;
using std::endl;

/*--------------------------------------------------------------------------
-- FUNCTION: WinTimer
--
-- DATE: Apr. 05, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Apr/05 - Created Funtions 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  WinTimer ()
--  
-- NOTES:
-- Ctor
--------------------------------------------------------------------------*/
WinTimer::WinTimer()
	:timer(NULL),reset(0)
{
	makeWinTimer();
}

/*--------------------------------------------------------------------------
-- FUNCTION: WinTimer
--
-- DATE: Apr. 05, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Apr/05 - Created Funtions 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  WinTimer ()
--  
-- NOTES:
-- Overloaded Ctor
--------------------------------------------------------------------------*/
WinTimer::WinTimer(const long long time, const long rst)
	: timer(NULL), reset(rst)
{
	setDueTime(time);
	makeWinTimer();
}

/*--------------------------------------------------------------------------
-- FUNCTION: steDueTime
--
-- DATE: APR. 05, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/05 - Created Function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void steDueTime (cont long long time)
-- cont long long time - The time in 100 nano seconds
--
-- NOTES:
-- Sets the timer 
--------------------------------------------------------------------------*/
inline void WinTimer::setDueTime(const long long time)
{
	dueTime.QuadPart = time;
}


/*--------------------------------------------------------------------------
-- FUNCTION: steDueTime
--
-- DATE: APR. 05, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/05 - Created Function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void steDueTime (cont long long time)
-- cont long long time - The time in 100 nano seconds
-- const long rst 	- The number of resets
-- NOTES:
-- Sets the timer with reset value
--------------------------------------------------------------------------*/	
void WinTimer::setTimer(const long long time, const long rst)
{
	dueTime.QuadPart = time;
	reset = rst;
}

/*--------------------------------------------------------------------------
-- FUNCTION: resetTimer
--
-- DATE: APR. 05, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/05 - Created Function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void resetTimer ()
--
-- NOTES:
-- Sets the timer with reset value
--------------------------------------------------------------------------*/	
int WinTimer::resetTimer()
{
	if (!SetWaitableTimer(timer, &dueTime, reset, NULL, NULL, 0))
	{
		printf("SetWaitableTimer failed (%d)\n", GetLastError());
		return 1;
	}
	return 0;
}

/*--------------------------------------------------------------------------
-- FUNCTION: makeWinTimer
--
-- DATE: APR. 05, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/APR/05 - Created Function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void makeWinTimer ()
--
-- NOTES:
-- make the timer structure
--------------------------------------------------------------------------*/
void WinTimer::makeWinTimer()
{
	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	if (NULL == timer)
	{
		cerr << "CreateWaitableTimer failed (%d)\n" 
			 << GetLastError()  << endl;
	}
}