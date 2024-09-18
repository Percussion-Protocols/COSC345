#include "Interface.h"
#include "utils.h"  // Include the header where clearScreen is declared
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <map>
#include <array>
#include "Clock.h"
#include "Interface.h"
#include "Audio_Engine.h"

// Declarations of a few strings which will be used multiple times

std::string mainMenu = "please select (using the adjacent key) an action from the options below, press SPACE to play and pause:\n-(1)Edit Sequence\n-(2)Set BPM\n-(3)Set Pattern Name\n-(4)Exit\n";
int BPM = 175;
bool sequenceSet = false;
std::string patternName = "New Pattern";
bool playing = false;


/**
* @brief Method to display the contents of the current sequence
*
* @param[in] sequence the currrent sequence, the string is the name of the sound, and the array is the pattern
* @return void
*/
void Interface::displaySequence(const std::map<std::string, std::array<bool, 8>>& sequence) {
    clearScreen();
    int index = 1;
    std::cout << "BPM: " << BPM << "      Pattern name: " << patternName << "\n";
    for (auto i = sequence.begin(); i != sequence.end(); i++) {
        std::cout << "-(" << index << ")";
        for (int n = 0; n < 40 - i->first.size(); n++) {
            std::cout << " "; //aligns all paterns accounting for different name lengths of sounds
        }
        std::cout << i->first << ": ";
        index++;
        for (bool value : i->second) {
            std::cout << (value ? "0" : "_");
        }
        std::cout << std::endl;
    }
}

void Interface::displayPlayerSequence(const std::map<std::string, std::array<bool, 8>>& sequence, int headIndex) {
    int index = 1;
    std::cout << "BPM: " << BPM << "      Pattern name: " << patternName << "\n";
    for (auto i = sequence.begin(); i != sequence.end(); i++) {
        std::cout << "-(" << index << ")";
        for (int n = 0; n < 40 - i->first.size(); n++) {
            std::cout << " "; //aligns all paterns accounting for different name lengths of sounds
        }
        std::cout << i->first << ": ";
        index++;
        for (bool value : i->second) {
            if (index - 1 == headIndex) {
                std::cout << "|";
            }
            if (value) {
                std::cout << "0";
            }
            else {
                std::cout << "_";
            }
        }
        std::cout << std::endl;
    }
}
/**
* @brief method to display the main menu when no sequence has been loaded
* @param sequence The currrent sequence, the string is the name of the sound, and the array is the pattern
* @return void
*/
void Interface::displayMainMenu(const std::map<std::string, std::array<bool, 8>>& sequence) {
    clearScreen();
    std::cout << "Welcome to CMDrum\n";
    if (sequenceSet) {
        displaySequence(sequence);
    }
    std::cout << mainMenu;
}

void Interface::displayPlayerMenu(const std::map<std::string, std::array<bool, 8>>& sequence, int index) {
    clearScreen();
    std::cout << "Welcome to CMDrum\n";
    if (sequenceSet) {
        displayPlayerSequence(sequence, index);
    }
    std::cout << mainMenu;
}
/**
* @brief Method to show the display for editing a specific sound
* @param keyAndVals; a pair, where the string is the key of the sound, and the array is the pattern of the sound.
* @return void
*/
void Interface::soundEditDisplay(const std::map<std::string, std::array<bool, 8>>& sequence, int editing, std::pair<std::string, std::array<bool, 8>>& keyAndVals) {
    clearScreen();
    int index = 1;
    std::cout << "BPM: " << BPM << "      Pattern name: " << patternName << "\n";
    for (auto i = sequence.begin(); i != sequence.end(); i++) {
        if (index != editing) {
            std::cout << "-(" << index << ")";
            for (int n = 0; n < 40 - i->first.size(); n++) {
                std::cout << " "; //aligns all paterns accounting for different name lengths of sounds
            }
            std::cout << i->first << ": ";

            for (bool value : i->second) {
                std::cout << (value ? "0" : "_");
            }
            std::cout << "\n";
        }
        else {
            // Displays the sequence we are editing:
            std::cout << "(->)";
            for (int n = 0; n < 40 - i->first.size(); n++) {
                std::cout << " "; //aligns all paterns accounting for different name lengths of sounds
            }
            std::cout << keyAndVals.first << ": ";
            for (bool i : keyAndVals.second) {
                if (i) {
                    std::cout << "0";
                }
                else {
                    std::cout << "_";
                }
            }
            std::cout << "\n";
        }
        index++;

    }
    std::cout << "Press Enter to save.\nEnter 1 - 8 to add or delete a sound in slots 1 - 8:\n" << std::endl;





}

/**
* @brief Method to edit the pattern associated with a sound
* @param[in] sequence the map which holds our sounds and their associated patterns.
* @param[in] keyndVals a pair containing the name of the sound whos patttern we will be editing, and its associated pattern
* @return void
*/
void Interface::editSound(std::map<std::string, std::array<bool, 8>>& sequence, std::pair<std::string, std::array<bool, 8>>& keyAndVals, int soundIndex) {
    soundEditDisplay(sequence, soundIndex, keyAndVals);
    // Convert from char to int
    while (true) {
        char ch = _getch();
        int toChange = ch - '0';
        if (ch == 13) {
            break;
        }
        int index = toChange - 1;
        if (toChange >= 1 && toChange <= 8) {
            // Convert char to int index
            keyAndVals.second[index] = !keyAndVals.second[index];
        }
        soundEditDisplay(sequence, soundIndex, keyAndVals);
    }
    // Updates the actual sequence
    auto a = sequence.find(keyAndVals.first);
    if (a != sequence.end()) {
        a->second = keyAndVals.second;
    }
}

/**
* @brief Helper method to find the key corresponding to a certain index in the map
* @param[in] soundIndex the index of the sound whos key we want,
* @param[in] sequence the currrent sequence, the string is the name of the sound, and the array is the pattern
* @returns a pair consisting of the name of the sound at the specified index, and an array representing the pattern
*/
std::pair<std::string, std::array<bool, 8>> Interface::getKeyByIndex(int soundIndex, const std::map<std::string, std::array<bool, 8>>& sequence) {
    int index = 1;
    for (auto i = sequence.begin(); i != sequence.end(); i++) {
        if (index == soundIndex) {
            return std::make_pair(i->first, i->second);
        }
        index++;
    }
    return std::make_pair("", std::array<bool, 8>{});
}

/**
* @brief Helper method to add a new sound to the sequence
* @param sequence; the currrent sequence, the string is the name of the sound, and the array is the pattern
* @return void
*/
void Interface::addSound(int newSound, std::map<std::string, std::array<bool, 8>>& sequence, Audio_Engine& E) {
    switch (newSound) {
    case 1:
        sequence.insert(std::make_pair("Kick 70s 1.wav", std::array<bool, 8>{false, false, false, false, false, false, false, false}));
        E.Preload("../Assets/Kick 70s 1.wav", "Kick 70s 1.wav");
        break;
    case 2:
        sequence.insert(std::make_pair("Snare 70s MPC 3.wav", std::array<bool, 8>{false, false, false, false, false, false, false, false}));
        E.Preload("../Assets/Snare 70s MPC 3.wav", "Snare 70s MPC 3.wav");
        break;
    case 3:
        sequence.insert(std::make_pair("Hihat Closed 80s UK Disco Vinyl.wav", std::array<bool, 8>{false, false, false, false, false, false, false, false}));
        E.Preload("../Assets/Hihat Closed 80s UK Disco Vinyl.wav", "Hihat Closed 80s UK Disco Vinyl.wav");
        break;
    }
}

/**
* @brief Method called when the user selects the edit sequence option. It will display the sequence editor, or if no sounds have been added, will prompt the user to add a sound.
* @param sequence the currrent sequence, the string is the name of the sound, and the array is the pattern
* @return int, just zero as this function does not return anything useful
*/
void Interface::editSequence(std::map<std::string, std::array<bool, 8>>& sequence, Audio_Engine& E) {
    clearScreen();
    int newSound;
    if (!sequenceSet) {
        std::cout << "No Sequence has been set. Please select your first sound to add to a new sequence:\n";
        std::cout << "which sound would you like to add?\n-(1)Kick\n-(2)Snare\n-(3)Hat\n";
        sequenceSet = true;
        char ch = _getch();
        while (true) {
            if (ch - '0' == 1) {
                newSound = ch - '0';
                addSound(newSound, sequence, E);
                return editSequence(sequence, E);
                break;
            }
            else if (ch - '0' == 2) {
                newSound = ch - '0';
                addSound(newSound, sequence, E);
                return editSequence(sequence, E);
                break;
            }
            else if (ch - '0' == 3) {
                newSound = ch - '0';
                addSound(newSound, sequence, E);
                return editSequence(sequence, E);
                break;
            }
            ch = _getch();
        }
    }
    displaySequence(sequence);
    int soundIndex;
    std::cout << "Which sound would you like to edit (enter the corresponding number)?\n(Or enter a to add a new sound, or Enter to save and exit to main menu)";
    char ch = _getch();
    soundIndex = ch - '0';
    if (ch == 13) {
        return;
    }
    if (ch == 97) {
        clearScreen();
        std::cout << "which sound would you like to add?\n-(1)Kick\n-(2)Snare\n-(3)Hat\n";
        char ch = _getch();
        while (true) {
            if (ch - '0' == 1) {
                newSound = ch - '0';
                addSound(newSound, sequence, E);
                return editSequence(sequence, E);
            }
            else if (ch - '0' == 2) {
                newSound = ch - '0';
                addSound(newSound, sequence, E);
                return editSequence(sequence, E);
            }
            else if (ch - '0' == 3) {
                newSound = ch - '0';
                addSound(newSound, sequence, E);
                return editSequence(sequence, E);
            }
            ch = _getch();
        }
        //std::cin >> newSound;
        //returns to editor
    }
    std::pair<std::string, std::array<bool, 8>> keyAndVals = getKeyByIndex(soundIndex, sequence);
    if (keyAndVals.first != "") {
        editSound(sequence, keyAndVals, soundIndex);
    }
    // Flip the value in the map of the index entered (minus 1 for zero indexing)
    return editSequence(sequence, E);
}

/**
* @brief function to play the sequence out loud
* @param[in] sequence the sequence we will be playing
* @returns int just zero, as this function does not return anyth
*/
void Interface::playSequence(const std::map<std::string, std::array<bool, 8>>& sequence, Audio_Engine& E) {
    //displayPlayerMenu(sequence, 0);
    displayMainMenu(sequence);
    Clock c = Clock::Clock(BPM);
    std::vector<std::string> names;
    std::vector<std::array<bool, 8>> patterns;
    for (auto i = sequence.begin(); i != sequence.end(); i++) {
         /*E.Preload("../Assets/" + i->first, i->first);*/
         names.push_back(i->first);
         patterns.push_back(i->second);

     }

    int index = 0;
    char ch = 'c';
    c.startClock();
    bool running = true;
    while (running) {
        if (c.interval()) {
            //std::vector<std::string> thisBeat; for multiple sounds
            //displayPlayerMenu(sequence, index);
            for (int i = 0; i < patterns.size() / sizeof(bool); i++) {
                if (patterns[i][index]) {
                    std::string name = names[i];
                    //thisBeat.push_back(name); for multiple sounds
                    //add sound into merge
                    E.PlaySound_(name);
                    //break;
                }
            }
            //E.mixDataNice(thisBeat); for multiple sounds
            index++;
        }
        //reset index when it reaches the end
        if (index > 7) {
            index = 0;
        }
        if (_kbhit()) {
            ch = _getch();
            if (ch == 32) {
                running = false;
            }

        }
    }
    playing = !playing;
    return;
}

/**
* @brief method to set the bpm based on user input
* @param[in] sequence, the current sequence
* @param[in] edit, a boolean used for recursive calls to tell the function if it should take input from typing or from the arrow keys
* @return void
*/
void Interface::setBPM(const std::map<std::string, std::array<bool, 8>>& sequence, bool edit) {
    clearScreen();
    displayMainMenu(sequence);
    if (edit) {
        std::cout << "Type in your bpm and press Enter to save:\n";
        std::cin >> BPM;
        if (BPM == 0) {
            std::cout << "\nBPM cannot be zero\n";
            setBPM(sequence, true);
        }
        return;
    }
    std::cout << "Press the up and down arrows to scroll bpm, or press E to type bpm.\nPress Enter to save\n";
    char ch = _getch();
    while (ch != 13) {
        if (ch == 101) {
            BPM = 0;
            return setBPM(sequence, true);
        }
        if (ch == 72) {
            BPM = BPM + 1;
            return setBPM(sequence, false);
        }
        if (ch == 80) {
            BPM = BPM - 1;
            return setBPM(sequence, false);
        }
        ch = _getch();
    }
    return;
}

/**
* @brief a method to name the pattern
*/
void Interface::setNewName() {
    clearScreen();
    std::cout << "Please enter a name for your pattern:\n";
    std::string name = "";
    while (name == "") {
        std::getline(std::cin, name);
    }
    std::cout << "Confirm name change to: \"" << name << "\" by pressing Enter, or cancel by pressing any other key";
    char ch = _getch();
    if (ch != 13) {
        return;
    }
    patternName = name;
    return;
}

/**
* @brief a method to perform an action based on user input
* @param[in] sequence, the seequence we will be performing actions on
* @param[in] E, the audio engine we will use to play sounds
* @return int, -1 if we should close the program, 0 otherwise
*/
int Interface::performAction(char choice, std::map<std::string, std::array<bool, 8>>& sequence, Audio_Engine& E) {
    bool exit = false;
    switch (choice) {
    case 49: {
        editSequence(sequence, E);
        break;
    }
    case 32: {
        playSequence(sequence, E);
        break;
    }
    case 50: {
        setBPM(sequence, false);
        break;
    }
    case 51: {
        setNewName();
        break;
    }
    case 52: {
        return -1;
        break;
    }
    default:
        std::cout << "Invalid choice. Please try again.\n";
    }
    return 0;
}


/**
* @brief class to perform unit tests
*/
int Interface::_test() {
    int status = 0;
    Audio_Engine E = Audio_Engine::Audio_Engine();
    // Test 1: Constructor
    try {
        Interface a;
    }
    catch (std::exception& e) {
        std::cout << "Interface constructor failed. Details: " << e.what() << std::endl;
        return 1;
    }

    // Test 2: getKeyByIndex
    try {
        Interface a;
        std::map<std::string, std::array<bool, 8>> testSequence;
        testSequence["Kick"] = { true, false, true, false, true, false, true, false };
        testSequence["Snare"] = { false, true, false, true, false, true, false, true };

        auto result = a.getKeyByIndex(1, testSequence);
        if (result.first != "Kick" || result.second != std::array<bool, 8>{true, false, true, false, true, false, true, false}) {
            throw std::runtime_error("getKeyByIndex returned unexpected result.");
        }
    }
    catch (std::exception& e) {
        std::cout << "getKeyByIndex test failed. Details: " << e.what() << std::endl;
        return 2;
    }

    // Test 3: addSound
    try {
        Interface a;
        std::map<std::string, std::array<bool, 8>> sequence;
        a.addSound(1, sequence, E);
        a.addSound(2, sequence, E);
        a.addSound(3, sequence, E);
        if (sequence.size() != 3 || sequence.find("Kick 70s 1.wav") == sequence.begin()) {
            throw std::runtime_error("addSound did not add the expected sound.");
        }
    }
    catch (std::exception& e) {
        std::cout << "addSound test failed. Details: " << e.what() << std::endl;
        return 3;
    }

    // Test 4: editSequence, then setBPM
    
    try {
        Interface a;
        std::map<std::string, std::array<bool, 8>> testSequence;
        a.performAction(49, testSequence, E);
        a.performAction(50, testSequence, E);
    }
    catch (std::exception& e) {
        std::cout << "setBPM test failed. Details: " << e.what() << std::endl;
        return 4;
    }
    

    // Test 5: performAction (exit)
    try {
        Interface a;
        std::map<std::string, std::array<bool, 8>> testSequence;
        int result = a.performAction('4', testSequence, E);
        if (result != -1) {
            throw std::runtime_error("performAction did not return expected value for exit action.");
        }
    }
    catch (std::exception& e) {
        std::cout << "performAction (exit) test failed. Details: " << e.what() << std::endl;
        return 5;
    }

    // Test 6: performAction (invalid action)
    try {
        Interface a;
        std::map<std::string, std::array<bool, 8>> testSequence;
        int result = a.performAction('9', testSequence, E);
        if (result != 0) {
            throw std::runtime_error("performAction did not return expected value for invalid action.");
        }
    }
    catch (std::exception& e) {
        std::cout << "performAction (invalid action) test failed. Details: " << e.what() << std::endl;
        return 6;
    }

    // Test 7: playSequence
    try {
        Interface a;
        std::map<std::string, std::array<bool, 8>> testSequence;
        playSequence(testSequence, E);
    }
    catch (std::exception& e) {
        std::cout << "playSequence didn't run as expected." << e.what() << std::endl;
        return 7;
    }

    std::cout << "All tests passed successfully." << std::endl;
    return 0;
}
