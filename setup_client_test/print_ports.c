/* Print out MIDI ports. */

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
    ItemCount n_devs = MIDIGetNumberOfDevices();
    printf("Number of devices: %d\n",(int)n_devs);
    while (n_devs--) {
        MIDIDeviceRef dev = MIDIGetDevice(n_devs);
        CFStringRef name = MIDIObjectRef_get_name(dev);
        printf(" Name of device %d: ",(int)n_devs);
        CFString_printf(name);
        printf("\n");
        ItemCount n_ent = MIDIDeviceGetNumberOfEntities(dev);
        printf(" Number of entities: %d\n",(int)n_ent);
        while (n_ent--) {
            MIDIEntityRef ent = MIDIDeviceGetEntity(dev,n_ent);
            CFStringRef name = MIDIObjectRef_get_name(ent);
            printf("  Name of entity %d: ",(int)n_ent);
            CFString_printf(name);
            printf("\n");
            ItemCount n_sources = MIDIEntityGetNumberOfSources(ent);
            printf("  Number of sources: %d\n",(int)n_sources);
            while (n_sources--) {
                MIDIEndpointRef source = MIDIEntityGetSource(ent,n_sources);
                CFStringRef name = MIDIObjectRef_get_name(source);
                printf("   Name of source %d: ",(int)n_sources);
                CFString_printf(name);
                printf("\n");
            }
            ItemCount n_dests = MIDIEntityGetNumberOfDestinations(ent);
            printf("  Number of destinations: %d\n",(int)n_dests);
            while (n_dests--) {
                MIDIEndpointRef dest = MIDIEntityGetDestination(ent,n_dests);
                CFStringRef name = MIDIObjectRef_get_name(dest);
                printf("   Name of destination %d: ",(int)n_dests);
                CFString_printf(name);
                printf("\n");
            }
        }
    }
    return 0;
}
