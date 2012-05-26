/* finddefault.c -- find_default_device() implementation
   Roger Dannenberg, June 2008
*/

#include <stdlib.h>
#include <string.h>
#include "portmidi.h"
#include "pmmacosxcm.h"
#include "readbinaryplist.h"

/* Parse preference files, find default device, search devices --
   This parses the preference file(s) once for input and once for
   output, which is inefficient but much simpler to manage. Note
   that using the readbinaryplist.c module, you cannot keep two
   plist files (user and system) open at once (due to a simple
   memory management scheme).
*/
PmDeviceID find_default_device(char *path, int input, PmDeviceID id)
/* path -- the name of the preference we are searching for
   input -- true iff this is an input device
   id -- current default device id
   returns matching device id if found, otherwise id
*/
{
    static char *pref_file = "com.apple.java.util.prefs.plist";
    char *pref_str = NULL;
    // read device preferences
    value_ptr prefs = bplist_read_user_pref(pref_file);
    if (prefs) {
        value_ptr pref_val = value_dict_lookup_using_path(prefs, path);
        if (pref_val) {
            pref_str = value_get_asciistring(pref_val);
        }
    }
    if (!pref_str) {
        bplist_free_data(); /* look elsewhere */
        prefs = bplist_read_system_pref(pref_file);
        if (prefs) {
            value_ptr pref_val = value_dict_lookup_using_path(prefs, path);
            if (pref_val) {
                pref_str = value_get_asciistring(pref_val);
            }
        }
    }
    if (pref_str) { /* search devices for match */
        int i;
        int n = Pm_CountDevices();
        /* first parse pref_str into name, interf parts */
        char *interf_pref = ""; /* initially assume it is not there */
        char *name_pref = strstr(pref_str, ", ");
        if (name_pref) { /* found separator, adjust the pointer */
            interf_pref = pref_str;
            /* modify the string to split into two parts. This write goes
               all the way into the prev_val data structure, but since
               noone else is going to use the string, it's ok to modify
               the data structure
             */
            interf_pref = pref_str;
            name_pref[0] = 0;
            name_pref += 2;
        } else {
            name_pref = pref_str; /* whole string is the name pattern */
        }
        for (i = 0; i < n; i++) {
            const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
            if (info->input == input &&
                strstr(info->name, name_pref) &&
                strstr(info->interf, interf_pref)) {
                id = i;
                break;
            }
        }
    }
    if (prefs) {
        bplist_free_data();
    }
    return id;
}
