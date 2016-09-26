#include "CaptureEMG.h"
#include <stdexcept>

CaptureEMG::CaptureEMG()
    : mHub("com.ponderousmad.MyoSnap")
{
    // Register self as listener.
    mHub.addListener(this);
}

// onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
void CaptureEMG::onUnpair(myo::Myo* myo, uint64_t timestamp)
{
}

// onEmgData() is called whenever a paired Myo has provided new EMG data, and EMG streaming is enabled.
void CaptureEMG::onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
{
    mSamples.push_back(Sample(timestamp, emg));
}

void CaptureEMG::print()
{
    for (auto i = mSamples.begin(); i != mSamples.end(); ++i) {
        i->print();
    }
    mSamples.clear();
}

bool CaptureEMG::setup() {
    try {
        // Grab the first connected myo, waiting up to 10 seconds.
        std::cout << "Attempting to find a Myo..." << std::endl;
        myo::Myo* myo = mHub.waitForMyo(10000);
        
        if (myo) {
            std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
            myo->setStreamEmg(myo::Myo::streamEmgEnabled);
            return true;
        } else {
            std::cout << "Unable to find a Myo!";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return false;
}

void CaptureEMG::update() {
    mHub.run(1);
    print();
}

CaptureEMG::Sample::Sample(uint64_t timestamp, const int8_t* emg)
    : mEmg()
    , mTimestamp(timestamp)
{
    for (int i = 0; i < 8; i++) {
        mEmg[i] = emg[i];
    }
}

void CaptureEMG::Sample::print() const
{
    std::cout << mTimestamp;
    for (size_t i = 0; i < mEmg.size(); i++) {
        std::cout << "," << static_cast<int>(mEmg[i]);
    }
    std::cout << std::endl;
}
