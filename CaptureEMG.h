#ifndef CaptureEMG_h
#define CaptureEMG_h

#include <myo/myo.hpp>

#include <array>
#include <vector>
#include <iostream>

class CaptureEMG : public myo::DeviceListener {
public:
    CaptureEMG();
    void onUnpair(myo::Myo* myo, uint64_t timestamp);
    void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg);
    void print();
    bool setup();
    void update();
private:
    class Sample {
    public:
        Sample(uint64_t timestamp, const int8_t* emg);
        void print() const;
    private:
        std::array<int8_t, 8> mEmg;
        uint64_t mTimestamp;
    };
    
    std::vector<Sample> mSamples;
    myo::Hub mHub;
};

#endif // CaptureEMG_h
