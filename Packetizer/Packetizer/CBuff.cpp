#include <iostream>
#include <mutex>

/*------------------------------------------------------------------------------
-- SOURCE FILE: CBuff.h - CircularBuffer Class
--
-- PROGRAM: COMP4985 A4
--
-- FUNCTIONS:
--  PUBLIC:
-- 		CBuff()
-- 		~CBuff()
-- 		void empty()
-- 		void push_back(std::string & str)
-- 		std::string & pop()
-- 		const std::string & peek() const
-- 		std::string & operator[] (int x)
-- 		const std::string & operator[] (int x) const
-- 		unsigned int getCapacity() const
-- 		bool isReadyForRead(double percentage = 0.5) const
--  PRIVATE:
-- 		unsigned int checkFull()
--	
-- DATE: MAR. 12, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MAR/12 - Create Class 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- Circular Buffer class that uses 
------------------------------------------------------------------------------*/

#include "CBuff.h"

#define OUT_OF_RANGE_ERROR std::out_of_range("ERROR: Out of Bounds")

using namespace std;

/*--------------------------------------------------------------------------
-- FUNCTION: CBuff
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE:  CBuff ()
--
-- NOTES:
-- Constructor
--------------------------------------------------------------------------*/
CBuff::CBuff()
	:_head(0),
	_tail(0),
	_buffSize(DEFAULT_SIZE),
	_capacity(DEFAULT_SIZE),
	_semHead(0),
	_semTail(DEFAULT_SIZE)
{
}

CBuff::~CBuff(){}

/*--------------------------------------------------------------------------
-- FUNCTION: empty()
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void empty () 
--
-- NOTES:
-- Releases the current array 
--------------------------------------------------------------------------*/
void CBuff::empty()
{
	_buff.empty();
}

/*--------------------------------------------------------------------------
-- FUNCTION: push_back
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void push_back (string & str)
-- string & str 
--
-- NOTES:
-- stores a string into the next available slot in the circular buffer
-- if the buffer is filled, the push will block until space becomes free
--------------------------------------------------------------------------*/	
void CBuff::push_back(string str)
{	
	// make sure you can insert, else wait
	_semTail.wait();
	_buff[ (_tail++) % _buffSize] = str;
	--_capacity;
	_semHead.signal();
}

/*--------------------------------------------------------------------------
-- FUNCTION: pop
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: void push_back (string & str)
-- string & str 
--
-- NOTES:
-- takes a string from the current slot pointed at by the head
-- in the circular buffer.
-- if the buffer is filled, the push will block until space becomes populated
--------------------------------------------------------------------------*/	
string & CBuff::pop()
{
	 _semHead.wait();
	string & temp = _buff[(_head++) % _buffSize];
	++_capacity;
	_semTail.signal();
	return temp;
}

/*--------------------------------------------------------------------------
-- FUNCTION: peek
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: cosnt string str peek () 
--
-- NOTES:
-- takes the next item from the space pointed to at by the buffer
-- but does nto remove it from the buffer 
--------------------------------------------------------------------------*/
const string & CBuff::peek() const
{
	return _buff[_tail];
}

/*--------------------------------------------------------------------------
-- FUNCTION: operator []
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: string & CBuff::operator[](int x)
--
-- NOTES:
-- [] operator overloaded 
--------------------------------------------------------------------------*/
string & CBuff::operator[](int x)
{
	if (static_cast<int>(_tail + x) < _buffSize - _capacity)
		return _buff[_tail + x];
	else
		throw OUT_OF_RANGE_ERROR;
}

/*--------------------------------------------------------------------------
-- FUNCTION: operator []
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: const string & CBuff::operator[](int x) const
--
-- NOTES:
-- [] operator overloaded 
--------------------------------------------------------------------------*/
const string & CBuff::operator[](int x) const
{
	if(static_cast<int>(_tail + x) < _buffSize - _capacity)
		return _buff[_tail + x];
	else
		throw OUT_OF_RANGE_ERROR;
}

/*--------------------------------------------------------------------------
-- FUNCTION: getCapacity
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: unsigned int getCapacity ()
--
-- RETURNS: 
-- unsigned int -- the space left in the buffer
--
-- NOTES:
-- returns the space left in the buffer
--------------------------------------------------------------------------*/
unsigned int CBuff::getCapacity() const
{
	return _capacity;
}

/*--------------------------------------------------------------------------
-- FUNCTION: isReadyForRead
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: bool isReadyForRead (double percentage)
-- double percentage -- value representing the percentage of buffer filled before the read
--
-- RETURNS: 
-- true or false represetenting if a buffer is ready to read
--
-- NOTES:
-- this can help when you want a specific number of packs
-- loaded before reading
--------------------------------------------------------------------------*/	
bool CBuff::isReadyForRead(double percentage) const
{
	return ( _head > _buffSize * percentage);
}

/*--------------------------------------------------------------------------
-- FUNCTION: checkFull
--
-- DATE: MM. DD, 2017
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MM/DD - DESCRIPTION 
--
-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- INTERFACE: bool checkFull ()
--
-- RETURNS: 
-- bool - true is full
--
-- NOTES:
-- checks if teh buffer is full 
--------------------------------------------------------------------------*/
unsigned int CBuff::checkFull() const
{
	if (_tail == _head)
	{
		return 1;
	}
	return 0;
}

