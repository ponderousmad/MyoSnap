#include "CaptureEMG.h"
#include "CaptureAudio.h"

#include <iostream>
#include <signal.h>

static volatile int sRunning = 1;

void intHandler(int) {
    sRunning = 0;
}

int main(int argc, char** argv)
{
    signal(SIGINT, intHandler);
    
    CaptureEMG emg;
    CaptureAudio audio;
    if (!emg.setup()) {
        std::cout << "EMG setup failed";
        return -1;
    }
    if (!audio.setup()) {
        std::cout << "Audio setup failed";
        return -2;
    }
    
    while(sRunning) {
        emg.update();
    }
    std::cout << "And done!" << std::endl << std::endl;
    return 0;
}
