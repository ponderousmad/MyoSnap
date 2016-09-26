#include "CaptureEMG.h"

int main(int argc, char** argv)
{
    CaptureEMG capture;
    if (capture.setup()) {
        while(1) {
            capture.update();
        }
    }
}
