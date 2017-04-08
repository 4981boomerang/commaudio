#pragma once
#include <windows.h>

#define RESET_DEFAULT 500
#define DUETIME_DEFAULT -1500000LL // 150ms

class WinTimer
{
public:
	WinTimer();
	explicit WinTimer(const long long time, const long rst);
	const HANDLE & getTimer(void) { return timer; };
	void setTimer(const long long time = DUETIME_DEFAULT, const long rst = RESET_DEFAULT);
	int resetTimer();
	int cancelTimer();
private:
	inline void setDueTime(const long long time);
	void makeWinTimer();

	// private data members:
	HANDLE timer;
	LARGE_INTEGER dueTime;
	long reset;
};