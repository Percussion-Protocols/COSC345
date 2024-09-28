#include "pch.h"
#include "CppUnitTest.h"
#include "../drum machine/Audio_Engine.cpp"
#include "../drum machine/Interface.cpp"
#include "../drum machine/Clock.cpp"
#pragma comment(lib, "winmm.lib")

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Mock functions

// Mock input for _getch
int inputs[] = {

	-32, 72, -32, 80, // Changing sound selection
	32, 32, // Play, then pause
	'a', '1', 8, '\r', 'a', '\r', // Change BPM, then cancel changing BPM in both ways.
	's', 'h', 'e', 'l', 'l', 'o', 8, '\r', // Change sequence name to hello
	'b', // Invalid character
	'1', // Add note to sequence
	'z' // Exit
};
int counter = -1;
// Mock _getch() function
int _getch() {
	counter++;
	return inputs[counter];
}

// Mock _kbhit() function
int _kbhit() {
	return 1;
}

namespace UnitTesting
{
	TEST_CLASS(UnitTesting)
	{
	public:
		TEST_METHOD(TestAudioEngine)
		{
			Audio_Engine E;
			E.Preload("../Assets/Kick 70s 1.wav", "Kick");
			try {
				E.Preload("incorrect filename", "Fail");
			} catch (const std::runtime_error err) {
				std::cout << "Preload failed as expected (Invalid filename)" << std::endl;
			}
			try {
				E.Preload("../../drum machine.sln", "Fail");
			}
			catch (const std::runtime_error err) {
				std::cout << "Preload failed as expected (Missing RIFF header)" << std::endl;
			}
			E.PlaySound_("Kick");
			E.tick();
		}
		TEST_METHOD(TestClock)
		{
			int bpm = 120;
			Clock c(bpm);
			c.setBPM(115);
			c.startClock();
			// If running infinitely, this test is failing. Check the interval function.
			bool isReached = false;
			while (!isReached) {
				isReached = c.interval();
			}
		}
		TEST_METHOD(TestInterface)
		{
			Audio_Engine Eng = Audio_Engine::Audio_Engine();
			Interface::E = &Eng;
			std::map<std::string, std::array<bool, 8>> testSequence;
			Interface::performAction('\r', testSequence);
			Interface::performAction('z', testSequence);
			Interface::performAction('a', testSequence);

		}
	};
}
