#include <windows.h>
#include <iostream>
#include <memory>
#include "Packetizer.h"

using namespace std;

/*--------------------------------------------------------------------------
-- FUNCTION: SoundFilePacketizer
--
-- DATE: Mar. 29, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Mar/29 - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  SoundFilePacketizer (int size)
-- int size -- size of each packet 
--
-- NOTES:
-- ctor
--------------------------------------------------------------------------*/
SoundFilePacketizer::SoundFilePacketizer(int size)
	:filesize(-1), fp(nullptr), packsize(size)
{
}

/*--------------------------------------------------------------------------
-- FUNCTION: ~SoundFilePacketizer
--
-- DATE: Mar. 29, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/Mar/29 - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  SoundFilePacketizer (int size)
-- int size -- size of each packet 
--
-- NOTES:
-- dtor
--------------------------------------------------------------------------*/

SoundFilePacketizer::~SoundFilePacketizer()
{
	if (fp)
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
-- Version 1.0 - [EY] - 2016/Mar/29 - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  SoundFilePacketizer (int size)
-- int size -- size of each packet 
--
-- NOTES:
-- reutrns the value of the next packet		
--------------------------------------------------------------------------*/
string SoundFilePacketizer::getNextPacket()
{

	if (static_cast<unsigned int>(packindex) >= vPack.size() - 1)
	{
		return "";
	}
	return vPack[packindex++];
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
-- INTERFACE: vectro<Packet *> makePacketsFromFile(File* pfile, int packsize)
-- FILE * pfile - pointer to hte file
-- int packsize - the size of each packet to make, in bytes
--
-- RETURNS: 
-- a vector of pointers to packets
--
-- NOTES:
-- this function creates a vector pf packets and passes it back 
--------------------------------------------------------------------------*/

void SoundFilePacketizer::makePacketsFromFile(const char * fpath /*CBuff*/)
{
	int read;
	char * pstr = nullptr;

	//empty a vector that may hold something else
	vPack.empty();

	pstr = (char *)malloc(packsize+1);
	if (!pstr)
	{
		cout << "Error on malloc for ptr. "
			<< GetLastError();
		free(pstr);
		fclose(fp);
		return;
	}
	
	openFile(fpath);
	if (!fp)
	{
		return;
	}

	calcFileSize();
	
	while (!feof(fp))
	{
	
		string str;
		read = fread(static_cast<void*>(pstr), sizeof(char), packsize, fp);
		if (ferror(fp))
		{
			cout << "Error on file read. "
				<< GetLastError();
			free(pstr);
			fclose(fp);
		}

		if (read > 0 && read < packsize)
		{
			memset(pstr + read, 0, packsize - read);
		}
		else
		{
			pstr[packsize] = '\0';
		}
		vPack.push_back(std::string(pstr));
	}

	free(pstr);
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
		cerr << "fopen Failed Error: " << errno << endl;
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
		std::cerr << "fclose Failed Error: " << GetLastError();
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
-- INTERFACE: logn getFileSize (FIEL * pfile)
-- FIEL * pfile the file pointer
--
-- RETURNS:
-- long repersenting the size of the file ( in BYTES )
--
-- NOTES:
-- gets the size of the file
--------------------------------------------------------------------------*/
void SoundFilePacketizer::calcFileSize()
{
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		cerr << "Error on fseek. " << GetLastError();
		return;
	}
	if ((filesize = ftell(fp)) < 0)
	{
		cerr << "Error on ftell. " << GetLastError();
		rewind(fp);
		return;
	}
	rewind(fp);
}


