#include "Clock.h"
#include <iostream>


bool Clock::second = false;
bool Clock::second_d = false;

int64_t Clock::swing = 50;

Clock::Clock(int tempo) {
	this->tempo_ms = (60000 / tempo) / 2;
}

Clock::~Clock() {

}


void Clock::startClock() {
	time = std::chrono::high_resolution_clock::now();
}

void Clock::setBPM(int BPM) {
	tempo_ms = (60000 / BPM) / 2;
}

int Clock::getSwing() {
	return swing;//static_cast<int>(swing);
}

void Clock::setSwing(int swing_) {

	swing = swing + swing_;// (swing > 0 && swing < 100 ? swing_ : 0);
	if (swing <= 0) swing = 0;
	if (swing > 100) swing = 100;
}

bool Clock::interval() {
	
	auto timeNow = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - time).count();
	
	if ((elapsed+0) >= (tempo_ms+msOffset)) {
		time = std::chrono::high_resolution_clock::now();
		second = !second;
		int64_t mult = tempo_ms / 85;
		int64_t ms = ((swing - 50))*mult; 
		msOffset = (second ? (ms) : ( - 1 * ms));
		return true;
	} else return false;
}

bool Clock::interval_d() {
	auto timeNow = std::chrono::high_resolution_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - time).count();

	if ((elapsed) >= (tempo_ms)) {
		time = std::chrono::high_resolution_clock::now();
		
		//msOffset_d = (second_d ? (tempo_ms * (swing - 50) / 50) : 0); //(second_d ? 50 : 0);//(second==true ? (tempo_ms / ((swing - 50) + 1)*20) : 0);
		//second_d = !second_d;
		return true;
	}
	else return false;
}
