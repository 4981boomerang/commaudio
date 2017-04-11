#include "stdafx.h"
#include "Packetizer.h"

#include <windows.h>
#include <iostream>
#include <memory>


using namespace std;

/*--------------------------------------------------------------------------
-- FUNCTION: SoundFilePacketizer
--
-- DATE: Mar. 29, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Mar/29 - Comment Added 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  SoundFilePacketizer (int size)
-- int size -- size of each packet, defaults to 1024 
--
-- NOTES:
-- ctor
--------------------------------------------------------------------------*/
SoundFilePacketizer::SoundFilePacketizer(int size)
	:filesize(-1), fp(nullptr), packsize(size), packindex(0)
{
}

/*--------------------------------------------------------------------------
-- FUNCTION: ~SoundFilePacketizer
--
-- DATE: Mar. 29, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Mar/29 - Comment aded 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  ~SoundFilePacketizer ()
--
-- NOTES:
-- dtor
--------------------------------------------------------------------------*/

SoundFilePacketizer::~SoundFilePacketizer()
{
	if (fp) // closes file if is is open 
	{
		closeFile();
	}
}

/*--------------------------------------------------------------------------
-- FUNCTION: SoundFilePacketizer
--
-- DATE: Mar. 29, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Mar/29 - Comment added
-- Version 1.0 - [EY] - 2016/Apr/09 - updated for audio file sending
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  getNextPacket ()
--
-- NOTES:
-- reutrns a pointer to the next packet (char * )
-- In order to not loose the data after you call makePacketsFromFile
-- you must do a memset on the data
--------------------------------------------------------------------------*/
char * SoundFilePacketizer::getNextPacket()
{
	if (static_cast<unsigned int>(packindex) < vPack.size())
	{
		return vPack[packindex++];
	}
	else
		return nullptr;
}

/*--------------------------------------------------------------------------
-- FUNCTION: getTotalPackets
--
-- DATE: Feb. 06, 2017
--
-- REVISIONS:
-- Version 1.0 - [EY] - 2016/Feb/06 - Created Functions
-- Version 1.1 - [EY] - 2016/Feb/06 - Turned into class funtion
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: long getTotalPackets (functionParams)
--
-- RETURNS:
-- long that represents the number of packets to send
--
-- NOTES:
-- calcs the number of packets to send when a file is broken down
-- to a specific byte suize per packet
--------------------------------------------------------------------------*/
long SoundFilePacketizer::getTotalPackets()
{
	return (filesize / packsize + (filesize % packsize != 0));
}

/*--------------------------------------------------------------------------
-- FUNCTION: getlastPackSize
--
-- DATE: Feb. 06, 2017
--
-- REVISIONS:
-- Version 1.0 - [EY] - 2016/Feb/06 - Created Functions
-- Version 1.1 - [EY] - 2016/Feb/06 - Turned into class funtion
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: long getTotalPackets ()
--
-- RETURNS:
-- long that represents the number of packets to send
--
-- NOTES:
-- calcs the number of packets to send when a file is broken down
-- to a specific byte size per packet
--------------------------------------------------------------------------*/
int SoundFilePacketizer::getLastPackSize()
{
	return static_cast<int>(filesize - ((getTotalPackets() - 1) * packsize));
}

/*--------------------------------------------------------------------------
-- FUNCTION: clearVector
--
-- DATE: Apr. 4, 2017
--
-- REVISIONS:
-- Version 1.0 - [EY] - 2016/Feb/06 - Clears out the vector by deleting all elements
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void clearVector()
--
-- NOTES:
-- clears out the vector. be warned! this is called every time you make 
-- a new file! 
--------------------------------------------------------------------------*/
void SoundFilePacketizer::clearVector()
{
	for (size_t i = 0; i < vPack.size(); ++i)
	{
		free(vPack[i]);
		vPack[i] = NULL;
	}
	filesize = 0;
	packindex = 0;
	vPack.clear();
}

/*--------------------------------------------------------------------------
-- FUNCTION: makePacketsFromFile(File* pfile, int packsize)
--
-- DATE: Feb. 06, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Feb/06 - Created Function 
-- Version 2.0 - [EY] - 2016/Feb/06 - Changed to be more compatible with current project
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void makePacketsFromFile(fpath)
-- const char * fpath -- the files path
--
-- RETURNS: 
-- a vector of pointers to packets
--
-- NOTES:
-- creates packets of *packsize* bytes for the entire files 
-- and stores it into a vector of the class
--------------------------------------------------------------------------*/

void SoundFilePacketizer::makePacketsFromFile(const char * fpath)
{
	size_t read;

	//empty a vector that may hold something else
	if (!vPack.empty())
	{
		clearVector();
	}
	
	openFile(fpath);
	if (!fp)
	{
		return;
	}

	calcFileSize();
	while (!feof(fp))
	{
	
		char * buff = static_cast<char *>(malloc(DEFAULT_PACKSIZE));
		read = fread(static_cast<void *>(buff), sizeof(char), packsize, fp);
		if (ferror(fp))
		{
			cout << "Error on file read. "
				<< GetLastError();
			fclose(fp);
		}
		vPack.push_back(buff);
		buff = nullptr;
	}
	closeFile();
}

/*--------------------------------------------------------------------------
-- FUNCTION: openFile
--
-- DATE: FEB. 06, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/FEB/06 - Created Function 
-- Version 2.0 - [EY] - 2016/FEB/06 - Changed to class; made safe with fopen_s 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: itn openFile (FILE * pfile)
-- FILE * pfile T-- file pointer
--
-- RETURNS: 
-- int representing state ( 0 is file opened , -1 unsuccessful)
--
-- NOTES:
-- wrapper function to open a file
--------------------------------------------------------------------------*/
void SoundFilePacketizer::openFile(const char * fpath)
{
	if ((fopen_s(&fp, fpath, "rb")) != 0)
	{
		cerr << "fopen Failed Error: " << 
			GetLastError() << endl;
	}
}

/*--------------------------------------------------------------------------
-- FUNCTION: closeFile
--
-- DATE: FEB. 06, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/FEB/06 - Created Function 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: itn closeFile (FILE * pfile)
-- FILE * pfile T-- file pointer
--
-- RETURNS: 
-- int representing state ( 0 is file opened , -1 unsuccessful)
--
-- NOTES:
-- wrapper function to close a file
--------------------------------------------------------------------------*/

void SoundFilePacketizer::closeFile()
{
	if (fclose(fp) != 0)
	{
		std::cerr << "fclose Failed Error: " 
			<< GetLastError() << endl;
		return;
	}
	fp = NULL;
}

/*--------------------------------------------------------------------------
-- FUNCTION: calcFileSize
--
-- DATE: Feb. 06, 2017
--
-- REVISIONS:
-- Version 1.0 - [EY] - 2016/Feb/06 - created function
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: logn calcFileSize ()
--
--
-- NOTES:
-- gets the size of the file and stores it in class member "filesize"
--------------------------------------------------------------------------*/
void SoundFilePacketizer::calcFileSize()
{
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		cerr << "Error on fseek. " << GetLastError() << endl;
		return;
	}

	if ((filesize = ftell(fp)) < 0)
	{
		cerr << "Error on ftell. " << GetLastError() << endl;
		rewind(fp);
		return;
	}

	rewind(fp);
}


