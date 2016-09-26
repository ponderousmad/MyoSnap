#include "CaptureAudio.h"

#include <iostream>

bool check(OSStatus error, const char* context)
{
    if (error) {
        std::cerr << "Error." << error << context << std::endl;
        return false;
    }
    return true;
}

// ____________________________________________________________________________________
// Determine the size, in bytes, of a buffer necessary to represent the supplied number
// of seconds of audio data.
int computeBufferSize(const AudioStreamBasicDescription *format, AudioQueueRef queue, float seconds)
{
    int packets, frames, bytes;
    frames = (int)ceil(seconds * format->mSampleRate);
    
    if (format->mBytesPerFrame > 0) {
        bytes = frames * format->mBytesPerFrame;
    } else {
        UInt32 maxPacketSize;
        if (format->mBytesPerPacket > 0) {
            maxPacketSize = format->mBytesPerPacket;	// constant packet size
        } else {
            UInt32 propertySize = sizeof(maxPacketSize);
            check(AudioQueueGetProperty(queue, kAudioConverterPropertyMaximumOutputPacketSize, &maxPacketSize,
                                                &propertySize), "couldn't get queue's maximum output packet size");
        }
        if (format->mFramesPerPacket > 0) {
            packets = frames / format->mFramesPerPacket;
        } else {
            packets = frames;	// worst-case scenario: 1 frame in a packet
        }
        if (packets == 0) { // sanity check
            packets = 1;
        }
        bytes = packets * maxPacketSize;
    }
    return bytes;
}

// ____________________________________________________________________________________
// AudioQueue callback function, called when a property changes.
void onPropertyChanged(void *userData, AudioQueueRef queue, AudioQueuePropertyID propertyID)
{
    CaptureAudio *capture = (CaptureAudio *)userData;
    capture->notePropertyChanged(propertyID);
}

// ____________________________________________________________________________________
// AudioQueue callback function, called when an input buffers has been filled.
static void onBuffer(void*                               userData,
                     AudioQueueRef                       queue,
                     AudioQueueBufferRef                 buffer,
                     const AudioTimeStamp*               startTime,
                     UInt32                              packets,
                     const AudioStreamPacketDescription* description)
{
    CaptureAudio *capture = (CaptureAudio *)userData;
    capture->logData(buffer->mAudioData, buffer->mAudioDataByteSize, packets, startTime->mHostTime);
    if (packets > 0) {
        capture->processAudio(queue, buffer, description, packets);
    }
}

// ____________________________________________________________________________________
// get sample rate of the default input device
OSStatus getSampleRate(Float64 *outSampleRate)
{
    OSStatus err;
    AudioDeviceID deviceID = 0;
    
    // get the default input device
    AudioObjectPropertyAddress addr;
    UInt32 size;
    addr.mSelector = kAudioHardwarePropertyDefaultInputDevice;
    addr.mScope = kAudioObjectPropertyScopeGlobal;
    addr.mElement = 0;
    size = sizeof(AudioDeviceID);
    err = AudioHardwareServiceGetPropertyData(kAudioObjectSystemObject, &addr, 0, NULL, &size, &deviceID);
    if (err)  {
        return err;
    }
    
    // get its sample rate
    addr.mSelector = kAudioDevicePropertyNominalSampleRate;
    addr.mScope = kAudioObjectPropertyScopeGlobal;
    addr.mElement = 0;
    size = sizeof(Float64);
    err = AudioHardwareServiceGetPropertyData(deviceID, &addr, 0, NULL, &size, outSampleRate);
    
    return err;
}

CaptureAudio::CaptureAudio(size_t buffers)
    : mBufferCount(buffers)
    , mVerbose(true)
    , mRunning(false)
{
}

bool CaptureAudio::setup()
{
    int i, bufferByteSize;
    AudioStreamBasicDescription recordFormat;
    CaptureAudio aqr;
    UInt32 size;
    
    // fill structures with 0/NULL
    memset(&recordFormat, 0, sizeof(recordFormat));
    memset(&aqr, 0, sizeof(aqr));

    // adapt record format to hardware and apply defaults
    if (recordFormat.mSampleRate == 0.) {
        check(getSampleRate(&recordFormat.mSampleRate), "Can't get sample rate");
    }
    
    if (recordFormat.mChannelsPerFrame == 0) {
        recordFormat.mChannelsPerFrame = 2;
    }
    
    if (recordFormat.mFormatID == 0 || recordFormat.mFormatID == kAudioFormatLinearPCM) {
        // default to PCM, 16 bit int
        recordFormat.mFormatID = kAudioFormatLinearPCM;
        recordFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
        recordFormat.mBitsPerChannel = 16;
        recordFormat.mBytesPerPacket = recordFormat.mBytesPerFrame =
        (recordFormat.mBitsPerChannel / 8) * recordFormat.mChannelsPerFrame;
        recordFormat.mFramesPerPacket = 1;
        recordFormat.mReserved = 0;
    }

    // create the queue
    if (!check(AudioQueueNewInput(
        &recordFormat,
        onBuffer,
        this /* userData */,
        NULL /* run loop */,
        NULL /* run loop mode */,
        0 /* flags */,
        &mQueue), "AudioQueueNewInput failed"))
    {
        return false;
    }
    
    // get the record format back from the queue's audio converter --
    // the file may require a more specific stream description than was necessary to create the encoder.
    size = sizeof(recordFormat);
    if(!check(AudioQueueGetProperty(mQueue, kAudioConverterCurrentOutputStreamDescription,
                                    &recordFormat, &size), "couldn't get queue's format")) {
        return false;
    }
    
    // allocate and enqueue buffers
    bufferByteSize = computeBufferSize(&recordFormat, mQueue, 0.1);	// enough bytes for half a second
    for (i = 0; i < mBufferCount; ++i) {
        AudioQueueBufferRef buffer;
        if (!check(AudioQueueAllocateBuffer(mQueue, bufferByteSize, &buffer), "AudioQueueAllocateBuffer failed")) {
            return false;
        }
        if (!check(AudioQueueEnqueueBuffer(mQueue, buffer, 0, NULL), "AudioQueueEnqueueBuffer failed")) {
            return false;
        }
    }
    
    // start the queue
    mRunning = true;
    return check(AudioQueueStart(mQueue, NULL), "AudioQueueStart failed");
}

void CaptureAudio::logData(void *buffer, UInt32 bytes, UInt32 packets, UInt64 timestamp) {
    if (mVerbose) {
        std::cout << "At " << timestamp;
        std::cout << " got buff data: " << buffer << " bytes: " << bytes << " packets: " << packets << std::endl;
    }
}

void CaptureAudio::processAudio(AudioQueueRef queue, AudioQueueBufferRef buffer, const AudioStreamPacketDescription *description, UInt32 packets) {
    
    /*
     Check(AudioFileWritePackets(aqr->recordFile, FALSE, inBuffer->mAudioDataByteSize,
     inPacketDesc, aqr->recordPacket, &inNumPackets, inBuffer->mAudioData),
     "AudioFileWritePackets failed");
     */
    mRecordPacket += packets;
    
    // if we're not stopping, re-enqueue the buffer so that it gets filled again
    if (mRunning) {
        check(AudioQueueEnqueueBuffer(queue, buffer, 0, NULL), "AudioQueueEnqueueBuffer failed");
    }
}

bool CaptureAudio::stopRecording() {
    // end recording
    std::cout << "* recording done *" << std::endl;
    mRunning = false;
    bool stopped = (AudioQueueStop(mQueue, TRUE), "AudioQueueStop failed");
    AudioQueueDispose(mQueue, TRUE);
    return stopped;
}

