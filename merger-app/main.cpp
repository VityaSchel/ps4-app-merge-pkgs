#include <sstream>
#include <iostream>
#include <orbis/libkernel.h>

#include "./common/log.h"

// Logging
std::stringstream debugLogStream;

int main(void)
{
    int sleepSeconds = 3;
    
    // No buffering
    setvbuf(stdout, NULL, _IONBF, 0);
    
    DEBUGLOG << "Hello world! Merger app is waiting " << sleepSeconds << " seconds!";
    sceKernelUsleep(2 * 1000000);
    DEBUGLOG << "Done. Infinitely looping...";

    for (;;) {}
}
