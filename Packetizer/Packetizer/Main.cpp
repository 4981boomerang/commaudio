#include <iostream>
#include <string>
#include <thread>

#include "Server.h"

void show_usage(const char * progName);

using namespace std;

int main(int argc, const char * argv[])
{
	Server server;
	server.startStream();
	Sleep(50000);
}

void show_usage(const char * progName)
{
	cout << "Usage: " << progName << " -s client_addr | -c \n";
	cout << "\t-s \t Server\n";
	cout << "\t-c \t Client" << endl;
}