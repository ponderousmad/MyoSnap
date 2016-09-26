#ifndef CaptureAudio_h
#define CaptureAudio_h

#include <AudioToolbox/AudioToolbox.h>

class CaptureAudio {
public:
    CaptureAudio(size_t buffers = 4);
    
    bool setup();
    void notePropertyChanged(AudioQueuePropertyID id) {}
    void logData(void *buffer, UInt32 bytes, UInt32 packets, UInt64 timestamp);
    void processAudio(AudioQueueRef queue, AudioQueueBufferRef buffer, const AudioStreamPacketDescription *description, UInt32 packets);
    bool stopRecording();
    
private:
    AudioQueueRef				mQueue;
    size_t                      mBufferCount;
    SInt64						mRecordPacket; // current packet number in record file
    bool						mRunning;
    bool						mVerbose;
};

#endif // CaptureAudio_h
