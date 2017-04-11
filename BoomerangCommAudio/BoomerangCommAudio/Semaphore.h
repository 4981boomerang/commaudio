/*------------------------------------------------------------------------------
-- SOURCE FILE: Semaphore.h - Semaphore class
--
-- PROGRAM: COMP4985 A5
--
-- FUNCTIONS:
-- 
--
-- DATE: MAR. 15, 2016
--
-- REVISIONS: 
-- Version 1.0 - [EY] - 2016/MAR/15 - DESCRIPTION 

-- DESIGNER: Eva Yu
--
-- PROGRAMMER: Eva Yu
--
-- NOTES:
-- Semaphore header class that will lock the circular buffer from
-- writing into the buffer or reading from the buffer until there is 
-- data or space available in the buffer
------------------------------------------------------------------------------*/
#include <iostream>
#include <mutex>
#include <condition_variable>

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

class Semaphore
{
public:
	explicit Semaphore(int count = 0)
		: _count(count) {}
	
	/*--------------------------------------------------------------------------
	-- FUNCTION: wait
	--
	-- DATE: Mar. 15, 2017
	--
	-- REVISIONS: 
	-- Version 1.0 - [EY] - 2016/Mar/15 - Created Function 
	--
	-- DESIGNER: Eva Yu
	--
	-- PROGRAMMER: Eva Yu
	--
	-- INTERFACE: inline void wait ()
	--  desc
	--
	-- RETURNS: 
	-- RETURN
	--
	-- NOTES:
	-- will decrease count by one if the value is negative, the user will 
	-- wait until a signal ( condition variable ) is met 
	--------------------------------------------------------------------------*/
	inline void wait() {
		std::unique_lock<std::mutex> lock(_mtx);
		while (!_count) {
			_condition.wait(lock);
		}
		--_count;
	}

	/*--------------------------------------------------------------------------
	-- FUNCTION: signal
	--
	-- DATE: Mar. 29, 2017
	--
	-- REVISIONS: 
	-- Version 1.0 - [EY] - 2016/Mar/29 - Created Function 
	--
	-- DESIGNER: Eva Yu
	--
	-- PROGRAMMER: Eva Yu
	--
	-- INTERFACE: inline void signal ()
	--
	-- RETURNS: 
	-- RETURN
	--
	-- NOTES:
	-- increments the count by one. it will signal something in the wait 
	-- state to proceed
	--------------------------------------------------------------------------*/
	inline void signal()
	{
		
		std::unique_lock<std::mutex> lock(_mtx);
		++_count;
		_condition.notify_one();
	}

private:
	std::mutex _mtx; // mutex used for locking resources 
	std::condition_variable _condition; // cv to be met ( lock )
	int _count; // the semaphore count 
};

#endif