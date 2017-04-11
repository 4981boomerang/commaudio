/***************************
Main is test site for 
server at the moment
		
     o   o
      )-(
     (O O)
      \=/
     .-"-.
    //\ /\\
  _// / \ \\_
 =./ {,-.} \.=
     || ||
     || ||    hjw
   __|| ||__  `97
  `---" "---'

Your patience is appreciated
***************************/
#include <iostream>
#include <string>
#include <thread>

#include "Server.h"

void show_usage(const char * progName);

using namespace std;

int main(int argc, const char * argv[])
{
	Server server;
	server.loadLibrary();
	server.startStream();
	Sleep(50000);
}

void show_usage(const char * progName)
{
	cout << "Usage: " << progName << " -s client_addr | -c \n";
	cout << "\t-s \t Server\n";
	cout << "\t-c \t Client" << endl;
}