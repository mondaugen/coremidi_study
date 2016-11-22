/* Open connection with destination (rather than setting up MIDI source) and send
 * timestamped messages
 */

#include <stdlib.h>
#include <stdio.h>
#include <CoreMIDI.h> 
#include <HostTime.h> 
#include <string.h> 
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>

#define STR_BUFSIZE 200 

#define ERR_EXIT(x)\
    fprintf(stderr,"Error %s\n",x);\
return -1;

typedef void (*sig_t) (int);

static volatile int done = 0;

static mach_timebase_info_data_t sTimebaseInfo;

void int_handle(int signum)
{
    done = 1;
}

char *CFString_strncpy(char *dest,
        CFStringRef str,
        size_t n)
{
    CFStringEncoding encoding = kCFStringEncodingUTF8;
    if (!CFStringGetCString(str, dest, n, encoding)) {
        dest = NULL;
    }
    return dest;
}

int CFString_cstr_strncmp(CFStringRef s1, char *s2, size_t n)
{
    char buf[STR_BUFSIZE];
    if (CFString_strncpy(buf, s1, STR_BUFSIZE) == NULL) {
        return -2;
    }
    return strncmp(buf,s2,n);
}

/* Print CFString to stdout */
void CFString_printf(CFStringRef str)
{
    char buffer[STR_BUFSIZE];
    CFStringEncoding encoding = kCFStringEncodingUTF8;
    const char *ptr = CFStringGetCStringPtr(str, encoding);
    if (ptr == NULL) {
        if (CFStringGetCString(str, buffer, STR_BUFSIZE, encoding)) {
            ptr = buffer;
        }
    }
    if (ptr) {
        printf("%s",ptr);
    }
}

/* Get MIDI object's name as CFStringRef */
CFStringRef MIDIObjectRef_get_name(MIDIObjectRef obj)
{
    CFStringRef name = NULL;
    OSStatus err;
    err = MIDIObjectGetStringProperty(obj,kMIDIPropertyName,&name);
    if (err) {
        return NULL;
    }
    return name;
} 

UInt64 nano_to_absolute(UInt64 nano)
{
    return nano * sTimebaseInfo.denom / sTimebaseInfo.numer;
}

int main(int argc, char **argv)
{
    // Get Timebase info
    (void) mach_timebase_info(&sTimebaseInfo);

    char desired_dest_[] = "FastTrack Pro";
    char *desired_dest;
    if (argc < 2) {
        printf("No device specified, using default output %s.\n", desired_dest_);
        desired_dest = desired_dest_;
    } else {
        desired_dest = argv[1];
    }

    ItemCount n_dests = MIDIGetNumberOfDestinations();
    int found = 0;
    MIDIEndpointRef desired_epr;
    while (n_dests--) {
        MIDIEndpointRef dest = MIDIGetDestination(n_dests);
        CFStringRef name = MIDIObjectRef_get_name(dest);
        if (CFString_cstr_strncmp(name,desired_dest,strlen(desired_dest))
                == 0) {
            printf(" Name of destination %d: ",(int)n_dests);
            CFString_printf(name);
            printf("\n");
            found = 1;
            desired_epr = dest;
        }
    }
    if (!found) {
        printf("Destination %s not found.\n",desired_dest);
        return -1;
    }
    OSStatus result;
    MIDIClientRef clientref;
    result = MIDIClientCreate(CFSTR("default"),NULL,NULL,&clientref);
    if (result < 0 ) {
        ERR_EXIT("Creating client.");
    }
    MIDIPortRef portref;
    result = MIDIOutputPortCreate(clientref,CFSTR("hiports"),&portref);
    if (result < 0 ) {
        ERR_EXIT("Creating port.");
    }
    signal(SIGINT,int_handle);
    while (!done) {
        // With multiple and time stamps
        ByteCount mpdsize = sizeof(MIDIPacketList)+sizeof(MIDIPacket)*20;
        char mpdata[mpdsize];
        memset(mpdata,0,mpdsize);
        MIDIPacketList *midipackets;
        midipackets = (MIDIPacketList*)mpdata;
        MIDIPacket *mp;
        mp = MIDIPacketListInit(midipackets);
        int n;
        UInt64 timeNano, timeScale;
        for (n = 0; n < 20; n += 2) {
            Byte noteOnData[3] = {0x90,60+(n/2),100};
            Byte noteOffData[3] = {0x80,60+(n/2),0};
            // on all the devices I tried, the timestamps seem to be ignored
            mp = MIDIPacketListAdd(midipackets,mpdsize,mp,
                    nano_to_absolute(((UInt64)n/2) * 100ULL * 1000ULL * 1000ULL),
                    // seems equivalent to 0
//                    0,
                    3,noteOnData);
            if (!mp) {
                ERR_EXIT("Adding MIDI packet.\n");
            }
            mp = MIDIPacketListAdd(midipackets,mpdsize,mp,
                    nano_to_absolute((((UInt64)n/2)+1) * 100ULL * 1000ULL * 1000ULL),
                    // seems equivalent to 0
//                    0,
                    3,noteOffData);
            if (!mp) {
                ERR_EXIT("Adding MIDI packet.\n");
            }
        }
        // Check packets
        printf("Number of packets: %d\n",(int)midipackets->numPackets);
        MIDIPacket *__p = &midipackets->packet[0];
        int i;
        for (i = 0; i < midipackets->numPackets; ++i) {
            printf("Timestamp: %llu\n"
                    "Length: %d\n"
                    "Data : ",
                    __p->timeStamp,
                    (int)__p->length);
            int j;
            for (j = 0; j < __p->length; j++) {
                printf("%d ",(int)__p->data[j]);
            }
            printf("\n");
            __p = MIDIPacketNext(__p);
        }
        printf("Sending notes\n");
        MIDISend(portref,desired_epr,midipackets);
        sleep(1);
    }

    MIDIPortDispose(portref);
    MIDIClientDispose(clientref);

    return 0;
}
