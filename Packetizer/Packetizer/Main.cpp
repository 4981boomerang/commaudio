#include <iostream>
#include <string>
#include <thread>
#include <windows.h>


#include "Server.h"
#include "Client.h"

void show_usage(const char * progName);

using namespace std;

int main(int argc, const char * argv[])
{
	if (argc < 2)
	{
		show_usage(argv[0]);
		exit(0);
	}
	switch (argv[1][1])
	{
	case 's':
		if (argc < 3)
		{
			show_usage(argv[0]);
			exit(0);
		}
		else
		{
		runServer(argv[2]);
		}
	case 'c':
		runClient();
	default:
		show_usage(argv[0]);
		exit(0);
	}
	system("pause");
}



void show_usage(const char * progName)
{
	cout << "Usage: " << progName << " -s client_addr | -c \n";
	cout << "\t-s \t Server\n";
	cout << "\t-c \t Client" << endl;
}