#include "CaptureAudio.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cmath>

const int SAMPLES_RATE = 44100;
const int SAMPLES_PER_BUFFER = 4096;
const int CHANNELS = 1;
const int BUFFER_SIZE = CHANNELS * SAMPLES_PER_BUFFER;
const float SAMPLE_MAX = 2 << 15;

using namespace std::chrono;

uint64_t getTimestamp() {
    microseconds ms = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch());
    return static_cast<uint64_t>(ms.count());
}

ALCdevice* setupCapture() {
    alGetError();
    ALCdevice* device = alcCaptureOpenDevice(NULL, SAMPLES_RATE, AL_FORMAT_MONO16, BUFFER_SIZE);
    if (alGetError() != AL_NO_ERROR) {
        return nullptr;
    }
    alcCaptureStart(device);
    return device;
}

ALint captureSamples(ALCdevice* device, ALshort* buffer) {
    ALint sampleCount = 0;
    alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, (ALCsizei)sizeof(ALint), &sampleCount);
    alcCaptureSamples(device, (ALCvoid*)buffer, sampleCount);
    return sampleCount;
}

void cleanupCapture(ALCdevice* device) {
    alcCaptureStop(device);
    alcCaptureCloseDevice(device);
}

CaptureAudio::CaptureAudio()
    : mDevice(nullptr)
    , mBuffer(new ALshort[BUFFER_SIZE])
{
}

CaptureAudio::~CaptureAudio() {
    delete[] mBuffer;
}

bool CaptureAudio::setup()
{
    mDevice = setupCapture();
    return mDevice != nullptr;
}

void CaptureAudio::logData(float energy, int samples, uint64_t timestamp) {
    std::cout << "A," << timestamp << "," << std::fixed << std::setprecision(4) << energy << "," << samples << std::endl;
}

void CaptureAudio::update() {
    if (!mDevice) {
        return;
    }
    ALint samples = captureSamples(mDevice, mBuffer);
    float noise = 0;
    if (samples) {
        for (int s = 0; s < samples; ++s) {
            noise += std::fabs(mBuffer[s] / SAMPLE_MAX);
        }
        logData(noise / samples, samples, getTimestamp());
    }
}

bool CaptureAudio::cleanup() {
    cleanupCapture(mDevice);
    mDevice = nullptr;
    return true;
}

