#ifndef CaptureAudio_h
#define CaptureAudio_h

#include <AudioToolbox/AudioToolbox.h>
#include "readerwriterqueue.h"

class CaptureAudio {
public:
    CaptureAudio(size_t buffers = 4);
    
    bool setup();
    void notePropertyChanged(AudioQueuePropertyID id) {}
    void logData(float energy, UInt32 packets, UInt64 timestamp);
    float processAudio(AudioQueueRef queue, AudioQueueBufferRef buffer, const AudioStreamPacketDescription *description, UInt32 packets);
    bool stopRecording();
    
    void update();
    
private:
    AudioQueueRef				mQueue;
    size_t                      mBufferCount;
    SInt64						mRecordPacket; // current packet number in record file
    bool						mRunning;
    
    moodycamel::ReaderWriterQueue<std::string> mLogData;
};

#endif // CaptureAudio_h
