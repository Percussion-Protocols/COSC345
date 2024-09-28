#pragma once
#include "Audio_Engine.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <assert.h>
#include <exception>
#include <map>
#include <array>
#include <algorithm>
#pragma comment(lib, "winmm.lib")

/**
 * Constructor class for audio engine. Initializes the audio device to write to, prepares audio buffers and inits the audio format
 *
 */
Audio_Engine::Audio_Engine() : hWaveOut(nullptr), voiceIndex(0), mixedBuffer(maxBufferSize) {

    // Initialize waveOut handle
    // Prepare the WAVEHDR structures
    // Open the waveform audio output device
    // Initialize a generic pcm wave format to use for headers
    WaveData wd;
    wd.waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    wd.waveFormat.nChannels = 2;  // stereo sound
    wd.waveFormat.nSamplesPerSec = 44100;  // 44.1 kHz sample rate
    wd.waveFormat.wBitsPerSample = 24;  // 24-bit
    wd.waveFormat.nBlockAlign = wd.waveFormat.nChannels * (wd.waveFormat.wBitsPerSample / 8);
    wd.waveFormat.nAvgBytesPerSec = wd.waveFormat.nSamplesPerSec * wd.waveFormat.nBlockAlign;

    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wd.waveFormat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        throw std::runtime_error("Failed to open waveform output device.");
    }

    for (int i = 0; i < NUM_BUFFERS; i++) {
        WAVEHDR waveHeader = {};
        waveHeader.lpData = nullptr;
        waveHeader.dwBufferLength = BUFFER_SIZE;
        waveHeader.dwFlags = 0;
        waveHeader.dwLoops = 0;

        /*if (waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
            waveOutClose(hWaveOut);
            throw std::runtime_error("Failed to prepare wave header.");
        }*/

        waveHeaders.push_back(waveHeader);
    }
}

/**
 * Destructor class frees up audio headers and closes waveout device
 *
 */
Audio_Engine::~Audio_Engine() { //needs tests is headers deallocate successfully
    /*
    for (auto& header : waveHeaders) {
        if (header.dwFlags & WHDR_PREPARED) {
            waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
        }
        if (header.lpData) {
            delete[] header.lpData;
        }

    }*/

    /*for (auto& header : waveHeaders) {
        waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
    }
    if (hWaveOut) {
        waveOutClose(hWaveOut);
    }*/
    if (hWaveOut) {
        waveOutReset(hWaveOut);  // Stop any playback

        for (auto& header : waveHeaders) {
            if (header.dwFlags & WHDR_PREPARED) {
                waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
            }
        }

        waveOutClose(hWaveOut);
        hWaveOut = nullptr;
    }

    mixedBuffer.clear();  // Free the memory used by mixedBuffer
    sounds_.clear();  // Clear the preloaded sounds

}
/**
 * Used to load sounds into memory at project init for playback later.
 *
 * \param filename the sound to preload into memory
 * \param id the identification to reference the sound data
 */
void Audio_Engine::Preload(const std::string& filename, const std::string& id) {
    sounds_[id] = LoadWave(filename);
    return;
}
void Audio_Engine::PlaySound_(std::string id) {


    auto data = sounds_.find(id);
    if (data == sounds_.end()) {
        throw std::runtime_error("Sound not found in mapping. Make sure to preload before playing a sound");
    }

    WaveData& waveData = data->second;

    WAVEHDR& currentHeader = waveHeaders[voiceIndex];
    currentHeader.lpData = reinterpret_cast<LPSTR>(waveData.audioData.data());
    currentHeader.dwBufferLength = static_cast<DWORD>(waveData.audioData.size());
    soundQueue.push(currentHeader);

    


}

void Audio_Engine::tick() {
    if (!soundQueue.empty()) {
        if (!(waveHeaders[voiceIndex].dwFlags & WHDR_DONE)) {
        
                    
            if (waveOutReset(hWaveOut) != MMSYSERR_NOERROR) { // stop waveform and reset waveform playhead to beginning
                std::cout << "could not stop playback successfully.";
                throw std::runtime_error("program has carked it");
            }
        
        }
        Process_(soundQueue);
        
    }
    
}
void Audio_Engine::Process_(std::queue<WAVEHDR>& q) {

    

    WAVEHDR* mixedHeader = &q.front(); //memeory leak

    
    if (q.size() > 0) {


        mixAudio(q, mixedHeader);
    }


    while (!q.empty()) {
        q.pop();
    }

    if (mixedHeader->dwBufferLength > 0) {
        if (waveOutPrepareHeader(hWaveOut, mixedHeader, sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
            waveOutWrite(hWaveOut, mixedHeader, sizeof(WAVEHDR));
        }

    }
    voiceIndex = (voiceIndex + 1) % NUM_BUFFERS;
    //waveOutUnprepareHeader(hWaveOut, &mixedHeader, sizeof(WAVEHDR));

}
void Audio_Engine::mixAudio(std::queue<WAVEHDR>& q, WAVEHDR* mixedHeader) {
    if (q.empty()) return;
    
    
    size_t maxLength = 0;
    std::vector<WAVEHDR> headers;

    // Process queue without moving data
    size_t queueSize = q.size();
    for (size_t i = 0; i < queueSize; ++i) {
        WAVEHDR& header = q.front();

        maxLength = (std::max)(maxLength, static_cast<size_t>(header.dwBufferLength));
        headers.push_back(header);

        q.push(header);  // Move to back of queue
        q.pop();  // Remove from front
    }

    std::fill(mixedBuffer.begin(), mixedBuffer.begin() + maxLength, 0);
    
    // Mix audio data
    for (const auto& header : headers) {
        for (DWORD i = 0; i < header.dwBufferLength; i += 3) { // Assuming 24-bit audio
            int32_t sample = 0;
            for (int j = 0; j < 3; ++j) {
                sample |= (static_cast<uint8_t>(header.lpData[i + j]) << (j * 8));
            }
            if (sample & 0x800000) sample |= 0xFF000000; // Sign extend

            int32_t mixedSample = 0;
            for (int j = 0; j < 3; ++j) {
                mixedSample |= (static_cast<uint8_t>(mixedBuffer[i + j]) << (j * 8));
            }
            if (mixedSample & 0x800000) mixedSample |= 0xFF000000; // Sign extend

            mixedSample += sample;

            // Clip the mixed sample
            mixedSample = (std::max)((std::min)(mixedSample, 8388607), -8388608);

            // Write back to mixed buffer
            for (int j = 0; j < 3; ++j) {
                mixedBuffer[i + j] = static_cast<char>((mixedSample >> (j * 8)) & 0xFF);
            }
        }
    }

    // Set up mixed header. make sure all entries are correct
    mixedHeader->lpData = mixedBuffer.data();
    mixedHeader->dwBufferLength = static_cast<DWORD>(maxLength);//maxLength;
    mixedHeader->dwFlags = 0;
    mixedHeader->dwLoops = 0;
    //delete[] mixedBuffer;
}

Audio_Engine::WaveData Audio_Engine::LoadWave(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open wave file.");
    }

    // Read RIFF header
    char riffHeader[4];
    file.read(riffHeader, 4);
    if (std::strncmp(riffHeader, "RIFF", 4) != 0) {
        throw std::runtime_error("Invalid wave file: missing RIFF header.");
    }

    file.ignore(4); // Skip file size bit

    // Read WAVE header
    char waveHeader[4];
    file.read(waveHeader, 4);
    if (std::strncmp(waveHeader, "WAVE", 4) != 0) {
        throw std::runtime_error("Invalid wave file: missing WAVE header.");
    }

    // Read fmt chunk
    char fmtHeader[4];
    file.read(fmtHeader, 4);
    if (std::strncmp(fmtHeader, "fmt ", 4) != 0) {
        throw std::runtime_error("Invalid wave file: missing fmt header.");
    }

    uint32_t fmtChunkSize;
    file.read(reinterpret_cast<char*>(&fmtChunkSize), 4);

    WaveData waveData;
    file.read(reinterpret_cast<char*>(&waveData.waveFormat), fmtChunkSize);

    // Read data chunk
    char dataHeader[4];
    file.read(dataHeader, 4);
    if (std::strncmp(dataHeader, "data", 4) != 0) {
        throw std::runtime_error("Invalid wave file: missing data header.");
    }

    uint32_t dataChunkSize;
    file.read(reinterpret_cast<char*>(&dataChunkSize), 4);

    waveData.audioData.resize(dataChunkSize);
    file.read(reinterpret_cast<char*>(waveData.audioData.data()), dataChunkSize);

    return waveData;
}


//#pragma once
//#include "Audio_Engine.h"
//#include <iostream>
//#include <fstream>
//#include <stdexcept>
//#include <cstring>
//#include <assert.h>
//#include <exception>
//#include <map>
//#include <array>
//#include <algorithm>
//#pragma comment(lib, "winmm.lib")
//
///**
// * Constructor class for audio engine. Initializes the audio device to write to, prepares audio buffers and inits the audio format
// *
// */
//Audio_Engine::Audio_Engine() : hWaveOut(nullptr), voiceIndex(0) {
//
//    // Initialize waveOut handle
//    // Prepare the WAVEHDR structures
//    // Open the waveform audio output device
//    // Initialize a generic pcm wave format to use for headers
//    WaveData wd;
//    wd.waveFormat.wFormatTag = WAVE_FORMAT_PCM;
//    wd.waveFormat.nChannels = 2;  // stereo sound
//    wd.waveFormat.nSamplesPerSec = 44100;  // 44.1 kHz sample rate
//    wd.waveFormat.wBitsPerSample = 24;  // 24-bit
//    wd.waveFormat.nBlockAlign = wd.waveFormat.nChannels * (wd.waveFormat.wBitsPerSample / 8);
//    wd.waveFormat.nAvgBytesPerSec = wd.waveFormat.nSamplesPerSec * wd.waveFormat.nBlockAlign;
//
//    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wd.waveFormat, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
//        throw std::runtime_error("Failed to open waveform output device.");
//    }
//
//    for (int i = 0; i < NUM_BUFFERS; i++) {
//        WAVEHDR waveHeader = {};
//        waveHeader.lpData = nullptr;
//        waveHeader.dwBufferLength = BUFFER_SIZE;
//        waveHeader.dwFlags = 0;
//        waveHeader.dwLoops = 0;
//
//       /* if (waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
//            waveOutClose(hWaveOut);
//            throw std::runtime_error("Failed to prepare wave header.");
//        }*/
//
//        waveHeaders.push_back(waveHeader);
//    }
//}
//
///**
// * Destructor class frees up audio headers and closes waveout device
// *
// */
//Audio_Engine::~Audio_Engine() { //needs tests is headers deallocate successfully
//    /*
//    for (auto& header : waveHeaders) {
//        if (header.dwFlags & WHDR_PREPARED) {
//            waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
//        }
//        if (header.lpData) {
//            delete[] header.lpData;
//        }
//
//    }*/
//
//    for (auto& header : waveHeaders) {
//        waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
//    }
//    if (hWaveOut) {
//        waveOutClose(hWaveOut);
//    }
//    
//}
///**
// * Used to load sounds into memory at project init for playback later.
// *
// * \param filename the sound to preload into memory
// * \param id the identification to reference the sound data
// */
//void Audio_Engine::Preload(const std::string& filename, const std::string& id) {
//    sounds_[id] = LoadWave(filename);
//    return;
//}
//void Audio_Engine::PlaySound_(std::string id) {
//
//    
//    auto data = sounds_.find(id);
//    if (data == sounds_.end()) {
//        throw std::runtime_error("Sound not found in mapping. Make sure to preload before playing a sound");
//    }
//
//    
//
//    /*WAVEHDR& currentHeader = waveHeaders[voiceIndex];
//    currentHeader.lpData = reinterpret_cast<LPSTR>(waveData.audioData.data());
//    currentHeader.dwBufferLength = static_cast<DWORD>(waveData.audioData.size());*/
//    soundQueue.push(data->second);
//    
//    
//    
//
//}
//
//void Audio_Engine::tick() {
//    if (!soundQueue.empty()) {
//        if (waveHeaders[voiceIndex].dwFlags & WHDR_DONE) {
//
//            
//            if (waveOutReset(hWaveOut) != MMSYSERR_NOERROR) { // stop waveform and reset waveform playhead to beginning
//                std::cout << "could not stop playback successfully.";
//                throw std::runtime_error("program has carked it");
//            }
//
//        }
//        Process_(soundQueue);
//        while (!soundQueue.empty()) {
//            soundQueue.pop();
//        }
//    }
//    
//    
//    //voiceIndex = 0;
//
//
//    /*for (auto& header : waveHeaders) {
//        if (header.dwFlags & WHDR_DONE) {
//            waveOutUnprepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
//            delete[] header.lpData;
//            header.lpData = nullptr;
//            header.dwFlags = 0;
//            std::cout << "unprepped sound." << std::endl;
//        }
//    }*/
//    
//        
//    //}
//    //clear contents
//    /*std::queue<WAVEHDR> empt;
//    std::swap(soundQueue, empt);*/
//    
//    
//    
//}
//void Audio_Engine::Process_(std::queue<WaveData> q) { //wavehdrs
//    
//    //WAVEHDR mixed;
//    //mixData(hWaveOut, q, &mixed);
//
//
//    //// Write data to the output device
//    //MMRESULT r = waveOutWrite(hWaveOut, &mixed, sizeof(WAVEHDR));
//    //if (r != MMSYSERR_NOERROR) {
//    //    printWaveOutError(r);  // Print the error message
//    //}
//    ////// Clean up the mixed buffer after it's been played
//    ////waveOutUnprepareHeader(hWaveOut, &mixed, sizeof(WAVEHDR));
//    ////delete[] mixed.lpData;
//    ////
// 
//    //if (q.empty()) return;
//
//    //WAVEHDR* mixedHeader = &q.front(); //memeory leak
//
//    /*char* copiedData = new char[mixedHeader.dwBufferLength];
//    std::memcpy(copiedData, mixedHeader.lpData, mixedHeader.dwBufferLength);
//    mixedHeader.lpData = copiedData;
//    delete copiedData;*/
//    //std::unique_ptr<WAVEHDR> mixedHeader = std::make_unique<WAVEHDR>(q.front());
//    if (q.size() > 0) {
//
//        
//        mixAudio(q, &waveHeaders[voiceIndex]); //edit this
//    }
//    //waveHeaders[voiceIndex].lpData = mixedHeader
//    
//    /*while (!q.empty()) {
//        q.pop();
//    }*/
//    
//    
//    /*if (mixedHeader->dwBufferLength > 0) {
//         if (waveOutPrepareHeader(hWaveOut, mixedHeader, sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
//            waveOutWrite(hWaveOut, mixedHeader, sizeof(WAVEHDR));
//        }
//        
//    }*/
//    if (waveHeaders[voiceIndex].dwBufferLength > 0) {
//        if (waveOutPrepareHeader(hWaveOut, &waveHeaders[voiceIndex], sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
//            MMRESULT r = waveOutWrite(hWaveOut, &waveHeaders[voiceIndex], sizeof(WAVEHDR));
//            if (r != MMSYSERR_NOERROR) {
//                std::cout << "error playing audio";
//                printWaveOutError(r);
//                throw std::runtime_error("program crashed playing audio");
//            }
//        }
//
//    }
//
//    //voiceIndex = (voiceIndex + 1) % NUM_BUFFERS;
//    //waveOutUnprepareHeader(hWaveOut, &mixedHeader, sizeof(WAVEHDR));
//    
//}
//void Audio_Engine::mixAudio(std::queue<WaveData> q, WAVEHDR* mixedHeader) { //address of q
//    if (q.empty()) return;
//
//    size_t maxLength = 0;
//    //std::vector<WaveData> waveDataVector;
//    std::vector<WAVEHDR> headers;
//
//    // Find the maximum buffer length and collect headers
//    while (!q.empty()) {
//        //WAVEHDR header = {};
//        //header.lpData = nullptr;
//        ////memcpy(header.lpData, reinterpret_cast<LPSTR>(q.front().audioData.data()), static_cast<DWORD>(q.front().audioData.size())); //header.lpData =  reinterpret_cast<LPSTR>(q.front().audioData.data());
//        //header.lpData = reinterpret_cast<LPSTR>(q.front().audioData.data());
//        //header.dwBufferLength = static_cast<DWORD>(q.front().audioData.size());
//        //waveDataVector.push_back(std::move(q.front()));
//        
//
//        WAVEHDR header = {};
//        header.lpData = reinterpret_cast<LPSTR>(q.front().audioData.data()); //waveDataVector.back()
//        header.dwBufferLength = static_cast<DWORD>(q.front().audioData.size());
//        header.dwFlags = 0;
//        header.dwLoops = 0;
//        maxLength = (std::max)(maxLength, static_cast<size_t>(header.dwBufferLength));
//        headers.push_back(header);
//        q.pop();
//    }
//    
//    // Allocate mixed buffer
//    //char* mixedBuffer = new char[maxLength]; //need to de-allocate this
//    //memset(mixedBuffer, 0, maxLength);
//    mixedBuffer.resize(maxLength);
//    std::fill(mixedBuffer.begin(), mixedBuffer.end(), 0);
//
//    // Mix audio data
//    for (const auto& header : headers) {
//        for (DWORD i = 0; i < header.dwBufferLength; i += 3) { // Assuming 24-bit audio
//            int32_t sample = 0;
//            for (int j = 0; j < 3; ++j) {
//                sample |= (static_cast<uint8_t>(header.lpData[i + j]) << (j * 8));
//            }
//            if (sample & 0x800000) sample |= 0xFF000000; // Sign extend
//
//            int32_t mixedSample = 0;
//            for (int j = 0; j < 3; ++j) {
//                mixedSample |= (static_cast<uint8_t>(mixedBuffer[i + j]) << (j * 8));
//            }
//            if (mixedSample & 0x800000) mixedSample |= 0xFF000000; // Sign extend
//
//            mixedSample += sample;
//
//            // Clip the mixed sample
//            mixedSample = (std::max)((std::min)(mixedSample, 8388607), -8388608);
//
//            // Write back to mixed buffer
//            for (int j = 0; j < 3; ++j) {
//                mixedBuffer[i + j] = static_cast<char>((mixedSample >> (j * 8)) & 0xFF);
//            }
//        }
//    }
//
//    // Set up mixed header. make sure all entries are correct
//    mixedHeader->lpData = mixedBuffer.data();
//    mixedHeader->dwBufferLength = static_cast<DWORD>(maxLength);//maxLength;
//    mixedHeader->dwFlags = 0;
//    mixedHeader->dwLoops = 0;
//    mixedBuffer.clear();
//    //delete[] mixedBuffer;
//}
//
//void Audio_Engine::printWaveOutError(MMRESULT error) {
//    // Buffer to store the error message
//    wchar_t errorText[MAXERRORLENGTH];
//
//    // Retrieve the error message corresponding to the MMRESULT error code
//    if (waveOutGetErrorText(error, errorText, MAXERRORLENGTH) == MMSYSERR_NOERROR) {
//        std::wstring wideStr(errorText);
//        std::string errorText(wideStr.begin(), wideStr.end());
//        std::cerr << "waveOutWrite error: " << errorText << std::endl;
//    }
//    else {
//        std::cerr << "waveOutWrite error: Unknown error code " << error << std::endl;
//    }
//    
//}
//
//Audio_Engine::WaveData Audio_Engine::LoadWave(const std::string& filename) {
//    std::ifstream file(filename, std::ios::binary);
//    if (!file) {
//        throw std::runtime_error("Failed to open wave file.");
//    }
//
//    // Read RIFF header
//    char riffHeader[4];
//    file.read(riffHeader, 4);
//    if (std::strncmp(riffHeader, "RIFF", 4) != 0) {
//        throw std::runtime_error("Invalid wave file: missing RIFF header.");
//    }
//
//    file.ignore(4); // Skip file size bit
//
//    // Read WAVE header
//    char waveHeader[4];
//    file.read(waveHeader, 4);
//    if (std::strncmp(waveHeader, "WAVE", 4) != 0) {
//        throw std::runtime_error("Invalid wave file: missing WAVE header.");
//    }
//
//    // Read fmt chunk
//    char fmtHeader[4];
//    file.read(fmtHeader, 4);
//    if (std::strncmp(fmtHeader, "fmt ", 4) != 0) {
//        throw std::runtime_error("Invalid wave file: missing fmt header.");
//    }
//
//    uint32_t fmtChunkSize;
//    file.read(reinterpret_cast<char*>(&fmtChunkSize), 4);
//
//    WaveData waveData;
//    file.read(reinterpret_cast<char*>(&waveData.waveFormat), fmtChunkSize);
//
//    // Read data chunk
//    char dataHeader[4];
//    file.read(dataHeader, 4);
//    if (std::strncmp(dataHeader, "data", 4) != 0) {
//        throw std::runtime_error("Invalid wave file: missing data header.");
//    }
//
//    uint32_t dataChunkSize;
//    file.read(reinterpret_cast<char*>(&dataChunkSize), 4);
//
//    waveData.audioData.resize(dataChunkSize);
//    file.read(reinterpret_cast<char*>(waveData.audioData.data()), dataChunkSize);
//
//    return waveData;
//}
//
//int Audio_Engine::_test() {
//    
//    int status = 0;
//    try { 
//        Audio_Engine a;
//
//    }
//    catch (std::exception& e) {
//        std::cout << "Audio engine constructor failed. Details: " << e.what();
//        return 1;
//    }
//    try { 
//        Audio_Engine a;
//        if (a.waveHeaders.size() == 0) {
//            throw std::exception("waveHeaders failed to initialize.");
//        }
//
//    }
//    catch (std::exception& e) {
//        std::cout << "Audio Engine waveHeaders has failed. Details: " << e.what();
//        return 2;
//    }
//    try { 
//        Audio_Engine a;
//        a.LoadWave("this will fail");
//        return 3;
//    }
//    catch (std::exception& e) {
//        std::cout << "Loadwave Failed Succesfully and as expected. ";
//    }
//
//    return 0;
//}