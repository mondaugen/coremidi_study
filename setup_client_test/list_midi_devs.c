// Setup a MIDI client and open a port. Test.

#include <stdio.h> 
#include <signal.h>
#include <unistd.h> 
#include <CoreAudio.h>
#include <CoreMIDI.h>

typedef void (*sig_t) (int);

static volatile int done = 0;

void int_handle(int signum)
{
    done = 1;
}

#define ERR_EXIT(x)\
    fprintf(stderr,"Error %s\n",x);\
    return -1;

int main (void)
{
    ItemCount nDevs;
    nDevs = MIDIGetNumberOfExternalDevices();
    MIDIDeviceRef dr;
    while (nDevs-- > 0) {
        dr = MIDIGetExternalDevice(nDevs);
        CFPropertyListRef pr;
        MIDIObjectGetProperties(dr,&pr,0);
    }
    return 0;
}

