#pragma once

#include <Winsock2.h>
#include <string>
#include <thread>

class RecvRoutine
{
private:
	SOCKET socket;
	bool isFile;
	FILE* file;
	std::string fileName;

public:
	RecvRoutine(SOCKET socket) : socket(socket) { isFile = false; }

	void RunThread();

};
