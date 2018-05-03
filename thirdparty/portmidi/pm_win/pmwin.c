/* pmwin.c -- PortMidi os-dependent code */

/* This file only needs to implement:
       pm_init(), which calls various routines to register the
           available midi devices,
       Pm_GetDefaultInputDeviceID(), and
       Pm_GetDefaultOutputDeviceID().
   This file must
   be separate from the main portmidi.c file because it is system
   dependent, and it is separate from, say, pmwinmm.c, because it
   might need to register devices for winmm, directx, and others.
 */

#include "stdlib.h"
#include "portmidi.h"
#include "pmutil.h"
#include "pminternal.h"
#include "pmwinmm.h"
#ifdef DEBUG
#include "stdio.h"
#endif

/* pm_exit is called when the program exits.
   It calls pm_term to make sure PortMidi is properly closed.
   If DEBUG is on, we prompt for input to avoid losing error messages.
 */
static void pm_exit(void) {
    pm_term();
#ifdef DEBUG
#define STRING_MAX 80
    {
        char line[STRING_MAX];
        printf("Type ENTER...\n");
        /* note, w/o this prompting, client console application can not see one
           of its errors before closing. */
        fgets(line, STRING_MAX, stdin);
    }
#endif
}


/* pm_init is the windows-dependent initialization.*/
void pm_init(void)
{
#ifdef DEBUG
    printf("registered pm_term with cleanup DLL\n");
#endif

#if 0
      // ??WS
#else
    atexit(pm_exit);
#ifdef DEBUG
    printf("registered pm_exit with atexit()\n");
#endif
#endif
    pm_winmm_init();
    /* initialize other APIs (DirectX?) here */
}


void pm_term(void) {
    pm_winmm_term();
}


PmDeviceID Pm_GetDefaultInputDeviceID() {
    /* This routine should check the environment and the registry
       as specified in portmidi.h, but for now, it just returns
       the first device of the proper input/output flavor.
     */
    int i;
    Pm_Initialize(); /* make sure descriptors exist! */
    for (i = 0; i < pm_descriptor_index; i++) {
        if (descriptors[i].pub.input) {
            return i;
        }
    }
    return pmNoDevice;
}

PmDeviceID Pm_GetDefaultOutputDeviceID() {
    /* This routine should check the environment and the registry
       as specified in portmidi.h, but for now, it just returns
       the first device of the proper input/output flavor.
     */
    int i;
    Pm_Initialize(); /* make sure descriptors exist! */
    for (i = 0; i < pm_descriptor_index; i++) {
        if (descriptors[i].pub.output) {
            return i;
        }
    }
    return pmNoDevice;

    // Prevent "unreachable code" warning
    // return 0;
}

#include "stdio.h"

void *pm_alloc(size_t s) {
    return malloc(s);
}


void pm_free(void *ptr) {
    free(ptr);
}

