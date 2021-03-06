/* Print sources and destinations (includes virtual ports) */

#include <stdlib.h>
#include <stdio.h>
#include <CoreMIDI.h> 

#define STR_BUFSIZE 200 

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

int main(void)
{
    ItemCount n_sources = MIDIGetNumberOfSources();
    while (n_sources--) {
        MIDIEndpointRef source = MIDIGetSource(n_sources);
        CFStringRef name = MIDIObjectRef_get_name(source);
        printf(" Name of source %d: ",(int)n_sources);
        CFString_printf(name);
        printf("\n");
    }
    ItemCount n_dests = MIDIGetNumberOfDestinations();
    while (n_dests--) {
        MIDIEndpointRef dest = MIDIGetDestination(n_dests);
        CFStringRef name = MIDIObjectRef_get_name(dest);
        printf(" Name of destination %d: ",(int)n_dests);
        CFString_printf(name);
        printf("\n");
    }
    return 0;
}
