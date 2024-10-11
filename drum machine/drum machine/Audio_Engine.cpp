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

        if (waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
            waveOutClose(hWaveOut);
            throw std::runtime_error("Failed to prepare wave header.");
        }

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
            if (header.lpData) {
                // delete[] header.lpData;
                header.lpData = nullptr;
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



    WAVEHDR* mixedHeader = &q.front();


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
    if (q.size() == 1) return;

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
    if (mixedBuffer.size() < maxLength) {
        mixedBuffer.resize(maxLength);
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

void Audio_Engine::printWaveOutError(MMRESULT error) {
    // Buffer to store the error message
    wchar_t errorText[MAXERRORLENGTH];

    // Retrieve the error message corresponding to the MMRESULT error code
    if (waveOutGetErrorText(error, errorText, MAXERRORLENGTH) == MMSYSERR_NOERROR) {
        std::wstring wideStr(errorText);
        std::string errorText(wideStr.begin(), wideStr.end());
        std::cerr << "waveOutWrite error: " << errorText << std::endl;
    }
    else {
        std::cerr << "waveOutWrite error: Unknown error code " << error << std::endl;
    }

}

Audio_Engine::WaveData Audio_Engine::LoadWave(const std::string& filename) {

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open wave file: " + filename);
    }

    WaveData waveData;
    char chunkID[4];
    uint32_t chunkSize;

    // Read RIFF header
    /* Flawfinder: ignore */
    file.read(chunkID, 4);

    if (std::strncmp(chunkID, "RIFF", 4) != 0) {
        throw std::runtime_error("Invalid WAVE file: RIFF header not found");
    }

    // The creator of flawfinder says this is how you should resolve this issue. See https://github.com/david-a-wheeler/flawfinder/issues/59
    /* Flawfinder: ignore */
    file.read(reinterpret_cast<char*>(&chunkSize), 4);
    /* Flawfinder: ignore */
    file.read(chunkID, 4);
    if (std::strncmp(chunkID, "WAVE", 4) != 0) {
        throw std::runtime_error("Invalid WAVE file: WAVE format not found");
    }

    bool formatChunkFound = false;
    bool dataChunkFound = false;

    while (file.good() && !(formatChunkFound && dataChunkFound)) {
        /* Flawfinder: ignore */
        file.read(chunkID, 4);
        /* Flawfinder: ignore */
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (std::strncmp(chunkID, "fmt ", 4) == 0) {
            if (chunkSize < 16) {
                throw std::runtime_error("Invalid format chunk size");
            }
            /* Flawfinder: ignore */
            file.read(reinterpret_cast<char*>(&waveData.waveFormat), 16);

            // Skip any extra format bytes
            file.ignore(chunkSize - 16);

            formatChunkFound = true;
        }
        else if (std::strncmp(chunkID, "data", 4) == 0) {
            waveData.audioData.resize(chunkSize);
            /* Flawfinder: ignore */
            file.read(reinterpret_cast<char*>(waveData.audioData.data()), chunkSize);

            dataChunkFound = true;
        }
        else {
            // Unknown chunk, skip it
            file.ignore(chunkSize);
        }

        // Ensure 2-byte alignment
        if (chunkSize % 2 != 0) {
            file.ignore(1);
        }
    }

    if (!formatChunkFound || !dataChunkFound) {
        throw std::runtime_error("Invalid WAVE file: Missing format or data chunk");
    }

    // Validate format
    if (waveData.waveFormat.wFormatTag != WAVE_FORMAT_PCM) {
        throw std::runtime_error("Unsupported WAVE format: Only PCM is supported");
    }

    if (waveData.waveFormat.nChannels == 0 || waveData.waveFormat.nSamplesPerSec == 0) {
        throw std::runtime_error("Invalid WAVE format: Invalid channel count or sample rate");
    }

    // Calculate nBlockAlign and nAvgBytesPerSec if they're not set correctly
    if (waveData.waveFormat.nBlockAlign != (waveData.waveFormat.nChannels * waveData.waveFormat.wBitsPerSample / 8)) {
        waveData.waveFormat.nBlockAlign = waveData.waveFormat.nChannels * waveData.waveFormat.wBitsPerSample / 8;
    }

    if (waveData.waveFormat.nAvgBytesPerSec != (waveData.waveFormat.nSamplesPerSec * waveData.waveFormat.nBlockAlign)) {
        waveData.waveFormat.nAvgBytesPerSec = waveData.waveFormat.nSamplesPerSec * waveData.waveFormat.nBlockAlign;
    }


    if (waveData.waveFormat.wBitsPerSample != 24) {
        convert24(waveData);

    }

    return waveData;
}
void Audio_Engine::Stop() {
    if (waveOutReset(hWaveOut) != MMSYSERR_NOERROR) { // Stop waveform and reset waveform playhead to beginning
        waveOutUnprepareHeader(hWaveOut, &waveHeaders[prevIndex], sizeof(WAVEHDR));
        std::cout << "Could not stop playback successfully.";
        throw std::runtime_error("Program has failed to reset playback.");
    }
}
void Audio_Engine::Preview(std::string filename) {

    {

        Stop(); //stop any playing audio
    }


    // Load the wave file
    WaveData wd = LoadWave(filename);

    size_t s = wd.audioData.size();
    previewBuf.clear();
    previewBuf.shrink_to_fit();
    previewBuf.resize(s);

    previewBuf = std::move(wd.audioData);
    /*waveHeader.lpData = nullptr;
    waveHeader.dwBufferLength = BUFFER_SIZE;
    waveHeader.dwFlags = 0;
    waveHeader.dwLoops = 0;*/
    // Set up the WAVEHDR for playback
    WAVEHDR& wh = previewHeader;//waveHeaders[voiceIndex];
    wh.lpData = reinterpret_cast<LPSTR>(previewBuf.data()); //audiobuf
    wh.dwBufferLength = static_cast<DWORD>(s);
    wh.dwFlags = 0;
    wh.dwLoops = 0;
    //wh.dwUser = reinterpret_cast<DWORD_PTR>(new bool(false));  // Use this to track when playback is complete

    // Prepare the header
    MMRESULT result = waveOutPrepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        //delete[] audioBuffer;
        delete reinterpret_cast<bool*>(wh.dwUser);
        printWaveOutError(result);
        throw std::runtime_error("Failed to prepare wave header.");
    }

    // Write to waveOut
    result = waveOutWrite(hWaveOut, &wh, sizeof(WAVEHDR));
    if (result != MMSYSERR_NOERROR) {
        waveOutUnprepareHeader(hWaveOut, &wh, sizeof(WAVEHDR));
        //delete[] audioBuffer;
        delete reinterpret_cast<bool*>(wh.dwUser);
        printWaveOutError(result);
        throw std::runtime_error("Failed to write to wave out.");
    }

    prevIndex = voiceIndex;
    voiceIndex = (voiceIndex + 1) % NUM_BUFFERS;




}

void Audio_Engine::convert24(WaveData& waveData) {
    if (waveData.waveFormat.wBitsPerSample == 24) {
        return;
    }

    if (waveData.waveFormat.wBitsPerSample != 16) {
        throw std::runtime_error("Only 16-bit to 24-bit conversion is supported.");
    }

    size_t oldSize = waveData.audioData.size();
    size_t newSize = oldSize * 3 / 2;

    // Create a new vector for the 24-bit data
    std::vector<BYTE> newAudioData;
    newAudioData.reserve(newSize);

    // Convert data
    for (size_t i = 0; i < oldSize; i += 2) {
        if (i + 1 >= oldSize) break; // ensure we have a full 16-bit sample

        int16_t sample16 = *reinterpret_cast<int16_t*>(&waveData.audioData[i]);
        int32_t sample24 = static_cast<int32_t>(sample16) << 8;

        newAudioData.push_back(static_cast<BYTE>(sample24 & 0xFF));
        newAudioData.push_back(static_cast<BYTE>((sample24 >> 8) & 0xFF));
        newAudioData.push_back(static_cast<BYTE>((sample24 >> 16) & 0xFF));
    }

    // Update the wave format
    waveData.waveFormat.wBitsPerSample = 24;
    waveData.waveFormat.nBlockAlign = waveData.waveFormat.nChannels * (waveData.waveFormat.wBitsPerSample / 8);
    waveData.waveFormat.nAvgBytesPerSec = waveData.waveFormat.nSamplesPerSec * waveData.waveFormat.nBlockAlign;

    // Replace the audio data
    waveData.audioData = std::move(newAudioData);

    //waveData.audioData.clear();
    //waveData.audioData.insert(waveData.audioData.end(), newAudioData.begin(), newAudioData.end());
}


void Audio_Engine::Unload(const std::string& id) {
    auto it = sounds_.find(id);

    if (it != sounds_.end()) {
        sounds_.erase(it);  // This removes the entry and frees associated memory
    }

}