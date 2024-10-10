#pragma once
#ifndef CLOCK_H
#define CLOCK_H
#include <chrono>
/*****************************************************************//**
 * \file   Clock.cpp
 * \brief  a class to handle clock timings for the sequencer
 *
 * \author Ben Addison, Alex Haszard
 * \date   August 2024
 *********************************************************************/

class Clock
{
public:
	/**
	 * Constructor for clock. sets the tempo of the drum machine
	 *
	 */
	Clock(int tempo);
	/**
	 * Destructor for clock.
	 *
	 */
	~Clock();
	/**
	 * Starts running the clock at the time of calling the function. Should be used when user plays the sequence
	 *
	 */
	void startClock();
	/**
	 * Sets the BPM for the clock to reference.
	 * 
	 * \param BPM - the desired beats per minute. 120 is is a good starter bpm
	 */
	void setBPM(int BPM);
	/**
	 * Signals when a time interval (beat); determined by the tempo is reached.
	 *
	 * \return bool - when a time interval has been met
	 */
	bool interval();
	/**
	 * signal time intervals but for displays.
	 * 
	 * \return true if signals
	 */
	bool interval_d();
	/**
	 * Test class for the clock.
	 *
	 * \return 0 - tests pass, 1 - tests fail
	 */
	static int test_();
	/**
	 * helper method to return the swing statically cast.
	 * 
	 * \return - da swing mate
	 */
	static int getSwing();
	/**
	 * sets the swing value for the clock.
	 * 
	 * \param swing - the swing value to assign
	 */
	static void setSwing(int swing_);

	static bool second;
	static bool second_d;
private:
	
	std::chrono::high_resolution_clock::time_point time; ///<the time measured from the system
	int64_t tempo_ms; ///< tempo converted into equivalent milliseconds
	static int64_t swing;

	int64_t msOffset = 0;
	int64_t msOffset_d = 0;
};
#endif


