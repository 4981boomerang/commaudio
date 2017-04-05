/*------------------------------------------------------------------------------
-- SOURCE FILE: timer.h - template headers for timer classes
--
-- PROGRAM: 5yearplan
--
-- FUNCTIONS:
-- Timer(); CTOR
-- ~Timer(); DSTOR
-- start();
-- stop();
-- getRandDuration();
--
-- DATE: NOV 12, 2016
--
-- REVISIONS:
-- Version 1.0 - [JA] - 2016/NOV/12 - created timer header
-- Version 2.0 - [JA] - 2016/NOV/16 - fixed timer headers to not take class and scoped function pointer
--
-- DESIGNER: John Agapeyev
--
-- PROGRAMMER: John Agapeyev
--
-- NOTES:
-- Timer classes here take an unsigned long long representing the number of milliseconds the timer will count for
------------------------------------------------------------------------------*/
#pragma once

#include <thread>
#include <random>
#include <chrono>
#include <condition_variable>


template<void(*ev)(), unsigned long duration>
class Timer {
	std::thread thread;
	std::mutex mutex;
	std::condition_variable cv;
	bool active = false;
	std::chrono::duration<unsigned long, std::milli> dur;

public:
	Timer() : dur(duration) {}
	~Timer() { stop(); }
	void start() {
		stop();
		{
			auto lock = std::unique_lock<std::mutex>(mutex);
			active = true;
		}
		thread = std::thread([&] {
			auto lock = std::unique_lock<std::mutex>(mutex);
			while (active) {
				auto result = cv.wait_for(lock, dur);
				if (result == std::cv_status::timeout) {
					ev();
					break;
				}
			}
		});
	}
	void stop() {
		{
			auto lock = std::unique_lock<std::mutex>(mutex);
			active = false;
		}
		cv.notify_one();
		if (thread.joinable()) {
			thread.join();
		}
	}
};

