#include "CaptureEMG.h"

#include <iostream>
#include <signal.h>

static volatile int sRunning = 1;

void intHandler(int) {
    sRunning = 0;
}

int main(int argc, char** argv)
{
    signal(SIGINT, intHandler);
    
    CaptureEMG capture;
    if (capture.setup()) {
        while(sRunning) {
            capture.update();
        }
    }
    std::cout << "And done!" << std::endl << std::endl;
}
