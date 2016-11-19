// Setup a MIDI client and open a port. Test.

#include <stdio.h> 
#include <signal.h>
#include <unistd.h> 
#include <CoreAudio.h>
#include <CoreMIDI.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <CoreServices.h> 
#include <stdint.h> 

typedef void (*sig_t) (int);

static volatile int done = 0;

void int_handle(int signum)
{
    done = 1;
}

#define ERR_EXIT(x)\
    fprintf(stderr,"Error %s\n",x);\
    return -1;

//uint64_t GetTimeInNanoseconds(void)
//{
//    uint64_t        time;
//    Nanoseconds     timeNano;
//
//    time = mach_absolute_time();
//
//    // Have to do some pointer fun because AbsoluteToNanoseconds 
//    // works in terms of UnsignedWide, which is a structure rather 
//    // than a proper 64-bit integer.
//
//    timeNano = AbsoluteToNanoseconds( *(AbsoluteTime *) &time );
//
//    return * (uint64_t *) &timeNano;
//}

int main (void)
{
    OSStatus result;
    MIDIClientRef clientref;
//    MIDIPortRef portref;
    MIDIEndpointRef epref;
    result = MIDIClientCreate(CFSTR("default"),NULL,NULL,&clientref);
    if (result < 0 ) {
        ERR_EXIT("Creating client.");
    }
    //result = MIDIOutputPortCreate(clientref,CFSTR("higuys"),&portref);
    //if (result < 0 ) {
    //    ERR_EXIT("Creating port.");
    //}
    result = MIDISourceCreate(clientref,CFSTR("higuys"),&epref);
    if (result < 0 ) {
        ERR_EXIT("Creating source.");
    }
    signal(SIGINT,int_handle);
//    char mpdata[sizeof(MIDIPacketList)+sizeof(MIDIPacket)];
//    MIDIPacketList *midipackets;
//    Byte data[] = {144,60,100};
//    midipackets = (MIDIPacketList*)mpdata;
//    midipackets->numPackets=1;
//    midipackets->packet[0].timeStamp = 0;
//    midipackets->packet[0].length = 3;
//    while (!done) {
////        printf("Note on\n");
//        midipackets->packet[0].data[0] = 0x90;
//        midipackets->packet[0].data[1] = 60;
//        midipackets->packet[0].data[2] = 100;
//        MIDIReceived(epref,midipackets);
//        usleep(50 * 1000);
////        printf("Note off\n");
//        midipackets->packet[0].data[0] = 0x80;
//        midipackets->packet[0].data[1] = 60;
//        midipackets->packet[0].data[2] = 0;
//        MIDIReceived(epref,midipackets);
//        usleep(50 * 1000);
//    }
    while (!done) {
        // With multiple and time stamps
        ByteCount mpdsize = sizeof(MIDIPacketList)+sizeof(MIDIPacket)*20;
        char mpdata[mpdsize];
        MIDIPacketList *midipackets;
        midipackets = (MIDIPacketList*)mpdata;
        MIDIPacket *mp;
        mp = MIDIPacketListInit(midipackets);
        int n;
        UInt64 timeNano, timeScale;
//        timeNano = GetTimeInNanoseconds();
        timeNano = 0;//AudioGetCurrentHostTime();
        timeScale = AudioConvertNanosToHostTime(1000 * 1000 * 1000);
        for (n = 0; n < 20; n += 2) {
            Byte noteOnData[3] = {0x90,60,100};
            Byte noteOffData[3] = {0x80,60,0};
            
            mp = MIDIPacketListAdd(midipackets,mpdsize,mp,
//                    timeNano + (n/2) * 1000 * 1000 * 1000,
                    timeNano + timeScale,
                    3,noteOnData);
            mp = MIDIPacketListAdd(midipackets,mpdsize,mp,
//                    timeNano + ((n/2)+1) * 1000 * 1000 * 1000,
                    timeNano + timeScale,
                    3,noteOffData);
        }
        printf("Sending notes\n");
        MIDIReceived(epref,midipackets);
        sleep(10);
    }
    MIDIEndpointDispose(epref);
    MIDIClientDispose(clientref);
    return 0;
}

