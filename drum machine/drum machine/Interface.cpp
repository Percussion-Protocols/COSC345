#include "Interface.h"
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <map>
#include <array>
#include <stdlib.h>

#include "Clock.h"
#include "Interface.h"
#include "Audio_Engine.h"

// Declarations of a few strings which will be used multiple times \x1b[90m- Command Line Drum Machine\x1b[97m 93

std::string mainMenu = "\n\x1b[90m'Enter' \x1b[93mView Editor\x1b[97m\n\x1b[90m  'P' \x1b[93m  Choose Number of Pages\n\x1b[90m  'Z' \x1b[93m  Exit\x1b[97m";
int BPM = 160;
bool sequenceSet = false;
std::string patternName = "New Pattern";
bool playing = false;

HANDLE Interface::hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
CHAR_INFO* Interface::screen = new CHAR_INFO[SCREEN_SIZE];
CHAR_INFO* Interface::bSounds = new CHAR_INFO[S_WIDTH * S_HEIGHT];

int Interface::windowX = 80;
int Interface::windowY = 40;

int Interface::sequenceLength = 8;
int Interface::pageNum = 0;

SHORT Interface::originX = 2;
SHORT Interface::originY = 4;

SMALL_RECT Interface::region = { 0,0,0,0 };

bool Interface::altView = false;

bool Interface::status = false;

int Interface::selection = 0;

int Interface::pIndex = 0;

bool Interface::exit_ = false;

Audio_Engine* Interface::E = nullptr;


void Interface::refresh() {
    printf("\x1b[" "?1049h"); //enter alternate buffer
    printf("\x1b[" "0;0f");
    printf("\x1b[" "2j");
    
    
}


void Interface::showEditor(std::map<std::string, std::vector<bool>>& sequence, int segment) {
    if (segment == -1) {
        //window, status bar etc
        refresh();
        

        drawBox({ originX, originY }, windowX, windowY);
        drawLine({ 2, 4 }, windowX, true, true);
        

        //pattern name
        printf("\x1b[" "%d;%dH", originX, originY);
        printf("\x1b[" "1B"); 
        printf("\x1b[" "13C"); 
        printf("\x1b[93mPattern:\x1b[0m %s", patternName.c_str());

        printf("\x1b[" "%d;%dH", originX + 3, originY + 2);
        printf("\x1b[96mTracks\x1b[0m");
        

        drawLine({ (SHORT)(windowX - 16),2 }, 2, false, true);

        printf("\x1b[" "%d;%df", 1, 4);
        printf("\x1b[90mBack to Menu:\x1b[0m 'Z'");

       
    }
    else if (segment == 1) {//sound list

        printf("\x1b[" "%d;%dH", originX + 5, originY + 7);
        int l = 0;
        for (auto i = sequence.begin(); i != sequence.end(); i++) {
            std::string s = i->first;
            if (s.length() > 15) {
                s.resize(15);
                s = s + "... .wav";
            }
            if (l == selection) {
                printf("\x1b[90m%d\x1b[0m  \x1b[107;30m%s\x1b[0;97m", l+1,s.c_str());
            }
            else {

                printf("\x1b[90m%d\x1b[0m  %s", l+1, s.c_str());
            }

            printf("\x1b[" "%dG", originY + 7);
            printf("\x1b[" "3B");

            l++;

        }
        drawLine({ (SHORT)(originX + 35), (SHORT)(originY + 1) }, l + 7, false, false);
    }
    else if (segment == 2) {//sequencer display
        // Any time we draw the sequencer display, we should update the page counter
        int offset = pageNum >= 9 ? 50 : 51;
        printf("\x1b[" "%d;%dH", originX + 1, originY + offset);
        printf("\x1b[93m Page:\x1b[0m %d/%d ", pageNum + 1, ((sequenceLength - 1) / 8) + 1);
        
        for (int j = 0; j < HEIGHT; j++) {
            for (int i = 0; i < WIDTH; i++) {
                screen[j * WIDTH + i].Attributes = WHITE | FOREGROUND_INTENSITY;
                
                screen[j * WIDTH + i].Char.UnicodeChar = 0x0020; 
                

            }
            
               
        }
        for (int i = 0; i < 8; i++) {
            screen[((i * 4)) + (i >= 4 ? 3 : 1)].Attributes = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
            screen[((i * 4)) + (i >= 4 ? 3 : 1)].Char.UnicodeChar = ('1' + i);
        }
        int i = 1;
        int x = 0;
        int y = 0;
        for (const auto& s : sequence) {

            for (int j = 0; j < 8; j++) {
                int buffer = pageNum * 8; // only show cells for current page, this puts a buffer at the beginning
                x = (i * (WIDTH * 3) - WIDTH);
                y = (1 + (j % 8) * 4) + ((j % 8) >= 4 ? 2 : 0);
                
                drawCell(x, y, s.second.at(j + buffer));
                if (status) {

                    drawPlayhead(pIndex % 16, sequence.size());
                }
                
            }
            i++;
        }
        //set up window for writing
        int hOffset = 40;
        int vOffset = 0;
        SMALL_RECT writeRegion = { originX + hOffset, originY + vOffset, originX + WIDTH + hOffset, originY + HEIGHT + vOffset };//{ 0 + hOffset, 0 + vOffset, 8 - 1 + hOffset, sequence.size() - 1 + vOffset };
        WriteConsoleOutput(hConsole, screen, { WIDTH,HEIGHT }, { 0,0 }, &writeRegion);
    }
    else if (segment == 3) { 
        printf("\x1b[" "%d;%df", 3, windowX-11 );
        if (status == false) {
            printf("Status:\x1b[91mStopped\x1b[0m");
        }
        else {
            printf("Status:\x1b[92mPlaying\x1b[0m");
        }
    }
    else if (segment == 4) { //bpm
        
        printf("\x1b[" "%d;%dH", originX + 1, originY + 2);
        printf("\x1b[93mBPM:\x1b[0m %d", BPM);
        drawLine({ 12,2 }, 2, false, true);
    }
        
   
}
void Interface::drawPlayhead(int i, int size) {
    for (int l = 0; l < ((size*4)-2); l++) {
        
        screen[(l * (WIDTH)) + ((i*2)+(i>=8 ? 3 : 1))].Char.UnicodeChar = 0x25B6;
        
    }
}

int Interface::getSequenceLength()
{
    return sequenceLength;
}

void Interface::setSequenceLength(int length)
{
    sequenceLength = length;
}

void Interface::nextPage()
{
    int numPages = ((sequenceLength - 1) / 8) + 1;
    pageNum = (pageNum + 1) % numPages;
}

void Interface::prevPage()
{
    int numPages = ((sequenceLength - 1) / 8) + 1;
    pageNum = (pageNum + (numPages-1)) % numPages;
}

void Interface::selectSound(std::map<std::string, std::vector<bool>>& sequence) {
    

    while (true) {
        char ch = _getch();
        if (ch == -32) {  
            ch = _getch();  
            if (ch == 72) { 
                selection = (selection - 1 + sequence.size()) % sequence.size();
            }
            else if (ch == 80) { 
                selection = (selection + 1 + sequence.size()) % sequence.size();
            }
            else if (ch == 'K') { // left arrow
                prevPage();
                showEditor(sequence, 2); //update sequence display
            }
            else if (ch == 'M') { // right arrow
                nextPage();
                showEditor(sequence, 2); //update sequence display
            }
            showEditor(sequence, 1); //update sound list display
        }
        else if (ch == 32) {
            
            
            status = !status;
            showEditor(sequence, 3); //playing status update
            playSequence(sequence);
            

        }
        else if (ch == 'a') {
            
            printf("\x1b[" "%d;%dH", windowY/3, windowX/2);
            printf("new bpm: ");
            
            char buf[4];
            std::fill(buf, buf + 4, '\0');
            int c = 0;
            bool confirm = false;
            while (!confirm) {
                char ch_ = _getch();
                if (c <= 2) { 

                    if (ch_ == '\r') break;
                    if (std::isdigit(ch_)) {

                        printf("%c", ch_);
                        buf[c] = ch_;
                        c++;
                    }
                }
                if (ch_ == '\r') break;
                if (ch_ == 8 && c >= 1) {
                    c--;
                    buf[c] = '\0';
                    printf("\x1b[" "1D");
                    printf("\x1b[" "1X");
                }

            }
            if (buf[0] != '\0') {

                int n = std::stoi(buf);
                BPM = (n <= 999 && n > 0 ? n : BPM);
            }
            
            
            showEditor(sequence, -1);
            showEditor(sequence, 1); 
            showEditor(sequence, 2); 
            showEditor(sequence, 3); 

            showEditor(sequence, 4); 
        }
        else if (ch == 's') { //pattern name
            printf("\x1b[" "%d;%dH", windowY / 3, windowX / 2);
            printf("new pattern name: ");
            const int maxSize = 20;
            char buf[maxSize];
            std::fill(buf, buf + maxSize, '\0');
            int c = 0;
            bool confirm = false;
            while (!confirm) {
                char ch_ = _getch();
                if (c <= maxSize-2 && ch_ != 8) { 

                    if (ch_ == '\r') break;
                    

                    printf("%c", ch_);
                    buf[c] = ch_;
                    c++;
                    
                }
                if (ch_ == '\r') break;
                if (ch_ == 8 && c >= 1) {
                    c--;
                    buf[c] = '\0';
                    printf("\x1b[" "1D");
                    printf("\x1b[" "1X");
                }
                

            }
            patternName = buf;

            showEditor(sequence, -1); 
            showEditor(sequence, 1); 
            showEditor(sequence, 2); 
            showEditor(sequence, 3); 

            showEditor(sequence, 4); 
        }
        else if (ch == 'z') {
            refresh();
            printf("\x1b[" "0;0f");
            displayMainMenu(sequence);
            break;
        }
        else {
            int cmd = (int)(ch - '0');
            if (cmd >= 1 && cmd <= 8) {
                int n = 0;
                int rowsPerSound = ((sequenceLength - 1) / 8) + 1;
                for (auto& i : sequence) {
                    if (n == selection) {
                        i.second[(8 * pageNum) + cmd - 1] = !i.second[(8 * pageNum)  + cmd - 1];
                    }
                    n++;
                }
            }
            showEditor(sequence, 2); //update sequence data
        }
    }
    
}

void Interface::drawCell(int v, int h, bool on) {
    if (on) {

        screen[v + h].Char.UnicodeChar = 0x2588;
    }
    else {
        screen[v + h].Char.UnicodeChar = 0x0020;
    }
    //tops bottoms
    screen[(v - WIDTH) + (h)].Char.UnicodeChar = 0x2500;
    screen[(v + WIDTH) + (h)].Char.UnicodeChar = 0x2500;

    //sides
    screen[(v) + (h+1)].Char.UnicodeChar = 0x2502;
    screen[(v) + (h-1)].Char.UnicodeChar = 0x2502;

    //corners
    screen[(v - WIDTH)+(h - 1)].Char.UnicodeChar = 0x250C;
    screen[(v - WIDTH) + (h + 1)].Char.UnicodeChar = 0x2510;

    screen[(v + WIDTH) + (h - 1)].Char.UnicodeChar = 0x2514;
    screen[(v + WIDTH) + (h + 1)].Char.UnicodeChar = 0x2518;

}

void Interface::drawBox(COORD pos, int sizeX, int sizeY) {
    int px = (int)pos.X;
    int py = (int)pos.Y;
    printf("\x1b" "(0");//line draw mode
    printf("\x1b[" "%d;%dH", px, py);
    for (int i = 0; i < sizeY; i++) { //vertical
        printf("\x1b[" "%d;%df", px+i, py);
        printf("x");
        printf("\x1b[" "%d;%df", px+i, py+sizeX);
        printf("x");
    }
    for (int i = 0; i < sizeX; i++) { //horizontal
        printf("\x1b[" "%d;%df", px + sizeY, py+i);
        printf("q");
        printf("\x1b[" "%d;%df", px, py+i);
        printf("q");
    }
    //corners
    printf("\x1b[" "%d;%dH", px, py);
    printf("l");
    printf("\x1b[" "%d;%df", px, py + sizeX);
    printf("k");
    printf("\x1b[" "%d;%df", px+sizeY, py + sizeX);
    printf("j");
    printf("\x1b[" "%d;%df", px + sizeY, py);
    printf("m");
    //reset cursor position to pos
    printf("\x1b[" "%d;%df", px, py);
    printf("\x1b" "(B"); // exit drawing mode

}
void Interface::drawLine(COORD pos, int length, bool horizontal, bool ends) {
    int px = (int)pos.Y;
    int py = (int)pos.X+3;
    printf("\x1b" "(0");//line draw mode
    printf("\x1b[" "%d;%dH", px, py);
    if (horizontal) {
        for (int i = 0; i < length-1; i++) { //horizontal
            
            printf("q");
        }
        printf("\x1b[" "%d;%df", px, py-1);

        if (ends) {    
            printf("t");
            printf("\x1b[" "%d;%df", px, py+length-1);
            printf("u");
        }
    }
    else {
        for (int i = 1; i < length; i++) { //vertical
            printf("\x1b[" "%d;%df", px+i, py);
            printf("x");
        }
        printf("\x1b[" "%d;%df", px, py);
        if (ends) {
            printf("w");
            printf("\x1b[" "%d;%df", px + length, py);
            printf("v");
        }
    }
    printf("\x1b[" "%d;%dH", px, py);
    printf("\x1b" "(B");
    
    
}

/**
* @brief method to display the main menu when no sequence has been loaded
* @param sequence The currrent sequence, the string is the name of the sound, and the array is the pattern
* @return void
*/
void Interface::displayMainMenu(const std::map<std::string, std::vector<bool>>& sequence) {
    
    
    std::cout << "\x1b[92mWelcome to CMDrum\x1b[97m \x1b[90m- Command Line Drum Machine\x1b[97m\n";
    
    

    /*if (sequenceSet) {
        displaySequence(sequence);
    }*/
    std::cout << mainMenu;
    
}

/**
* @brief Helper method to add a new sound to the sequence
* @param sequence; the currrent sequence, the string is the name of the sound, and the array is the pattern
* @return void
*/
void Interface::addSound(int newSound, std::map<std::string, std::vector<bool>>& sequence) {
    switch (newSound) {
    case 1:
        sequence.insert(std::make_pair("Kick 70s 1.wav", std::vector<bool>{}));
        for (int i = 0; i < sequenceLength; i++) {
            sequence["Kick 70s 1.wav"].push_back(false);
        }
        E->Preload("../Assets/Kick 70s 1.wav", "Kick 70s 1.wav");
        break;
    case 2:
        sequence.insert(std::make_pair("Snare 70s MPC 3.wav", std::vector<bool>{}));
        for (int i = 0; i < sequenceLength; i++) {
            sequence["Snare 70s MPC 3.wav"].push_back(false);
        }
        E->Preload("../Assets/Snare 70s MPC 3.wav", "Snare 70s MPC 3.wav");
        break;
    case 3:
        sequence.insert(std::make_pair("Hihat Closed 80s UK Disco Vinyl.wav", std::vector<bool>{}));
        for (int i = 0; i < sequenceLength; i++) {
            sequence["Hihat Closed 80s UK Disco Vinyl.wav"].push_back(false);
        }
        E->Preload("../Assets/Hihat Closed 80s UK Disco Vinyl.wav", "Hihat Closed 80s UK Disco Vinyl.wav");
        break;
    }
}


/**
* @brief function to play the sequence out loud
* @param[in] sequence the sequence we will be playing
* @returns int just zero, as this function does not return anyth
*/
void Interface::playSequence(std::map<std::string, std::vector<bool>>& sequence) {   
    Clock c = Clock::Clock(BPM);
    Clock display(BPM * 2);
    std::vector<std::string> names;
    std::vector<std::vector<bool>> patterns;
    for (auto i = sequence.begin(); i != sequence.end(); i++) {
         
         names.push_back(i->first);
         patterns.push_back(i->second);

     }

    pageNum = 0; // Go to the first page

    int index = 0;
    char ch = 'c';
    c.startClock();
    display.startClock();
    bool running = true;
    
    while (running) {
        if (display.interval()) {
            showEditor(sequence, 2);
            pIndex = (pIndex + 1) % (sequenceLength * 2);

            if (pIndex % 16 == 0) nextPage();
        }

        if (c.interval()) {
            for (int i = 0; i < patterns.size() / sizeof(bool); i++) {
                if (patterns[i][index]) {
                    std::string name = names[i];
                    E->PlaySound_(name);
                }
            }
            E->tick();
            index++;
        }
        
        if (index > sequenceLength-1) {
            index = 0;
        }

        if (_kbhit()) {
            ch = _getch();
            if (ch == 32) {
                running = false;
                status = !status;
                pIndex = 0;
                showEditor(sequence, 2);
                showEditor(sequence, 3);
            }
        }
    }

    return;
}

/**
* @brief a method to perform an action based on user input
* @param[in] sequence, the seequence we will be performing actions on
* @param[in] E, the audio engine we will use to play sounds
* @return int, -1 if we should close the program, 0 otherwise
*/
int Interface::performAction(char choice, std::map<std::string, std::vector<bool>>& sequence) {
    bool exit = false;
    switch (choice) {
    case '\r': { //1
        //start with 3 sounds
        addSound(1, sequence);
        addSound(2, sequence);
        addSound(3, sequence);


        showEditor(sequence, -1); //init
        showEditor(sequence, 1); //sound list
        showEditor(sequence, 2); //sequencer
        showEditor(sequence, 3); //status
        showEditor(sequence, 4); //bpm
        //edit the sequence
        selectSound(sequence);
        break;
    }
    case 'p': {
        printf("\x1b[" "%d;%dH", windowY / 3, windowX / 2);
        printf("Number of Pages (default 1, max 99): ");

        char buf[4];
        std::fill(buf, buf + 4, '\0');
        int c = 0;
        bool confirm = false;
        while (!confirm) {
            char ch_ = _getch();
            if (c <= 1) {

                if (ch_ == '\r') break;
                if (std::isdigit(ch_)) {

                    printf("%c", ch_);
                    buf[c] = ch_;
                    c++;
                }
            }
            if (ch_ == '\r') break;
            if (ch_ == 8 && c >= 1) {
                c--;
                buf[c] = '\0';
                printf("\x1b[" "1D");
                printf("\x1b[" "1X");
            }

        }
        if (buf[0] != '\0') {

            int n = std::stoi(buf);
            setSequenceLength(n <= 99 && n > 0 ? n * 8 : getSequenceLength());
        }
        refresh();
        displayMainMenu(sequence);
        break;
    }
    case 'z': { //leave
        refresh();
        std::cout << "\x1b[92mHave a good day.\x1b[97m";
        Sleep(1000);
        return -1;
        break;
    }
    default:
        refresh();
        std::cout << "\x1b[91mInvalid choice. Please try again.\x1b[97m\n";
        Sleep(750);
        refresh();
        displayMainMenu(sequence);
        

    }
    return 0;
}