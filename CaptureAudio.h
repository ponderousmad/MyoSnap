#ifndef CaptureAudio_h
#define CaptureAudio_h

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <cstdint>

class CaptureAudio {
public:
    CaptureAudio();
    ~CaptureAudio();
    
    bool setup();
    void logData(float energy, int samples, uint64_t timestamp);
    bool cleanup();
    
    void update();
    
private:
    ALCdevice*  mDevice;
    ALshort*     mBuffer;
};

#endif // CaptureAudio_h
