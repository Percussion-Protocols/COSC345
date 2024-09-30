// drum machine.cpp : This is the 'main' function
//

/*! \mainpage CMDrum Docs
 *
 * \section intro_sec Introduction
 *
 * CMDrum is a simple command-line drum sequencer.
 * 
 * To view details about the structure and functionality of the source code, <a href="./annotated.html">view the class list.</a>
 */
#include <vector>   
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include "Clock.h"
#include "Interface.h"
#include <map>

#pragma comment(lib, "winmm.lib")

int main() {

    std::map<std::string, std::vector<bool>> sequence;
    Audio_Engine Eng = Audio_Engine::Audio_Engine();
    Interface::E = &Eng;
    Interface::setSequenceLength(8);
    bool playing = false;
    int choice = 0;
    Interface::displayMainMenu(sequence);
    while (choice != 5) {
        char ch = _getch();
        int newBPM = Interface::performAction(ch, sequence); //E
        if (newBPM == -1) {
            choice = 5;
        }
    }   
    return 0;
}
