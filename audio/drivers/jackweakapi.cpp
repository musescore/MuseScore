//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  jackWeakAPI based on code from Stéphane Letz (Grame)
//  partly based on Julien Pommier (PianoTeq : http://www.pianoteq.com/) code.
//
//  Copyright (C) 2002-2019 Werner Schweer and others
//  Copyright (C) 2009 Grame

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation; either version 2.1 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.

//  You should have received a copy of the GNU Lesser General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <iostream>
#include <math.h>
#include <stdlib.h>
#if (defined (_MSCVER) || defined (_MSC_VER))
   // Include stdint.h and #define _STDINT_H to prevent <systemdeps.h> from redefining types
   // #undef UNICODE to force LoadLibrary to use the char-based implementation instead of the wchar_t one.
   #include <stdint.h>
   #define _STDINT_H 1  
#endif

#include <jack/jack.h>
#include <jack/midiport.h>
#include <jack/thread.h>
#include <jack/session.h>
#ifndef WIN32
#include <dlfcn.h>
//for backward compatibility of Jack headers. Might not be necessary.
typedef pthread_t jack_native_thread_t;
#endif

using std::cerr;

/* dynamically load libjack and forward all registered calls to libjack
   (similar to what relaytool is trying to do, but more portably..)
*/

typedef void (*print_function)(const char *);
typedef void *(*thread_routine)(void*);

static int libjack_is_present = 0;     // public symbol, similar to what relaytool does.

#ifdef WIN32
static HMODULE libjack_handle = 0;
#else
static void *libjack_handle = 0;
#endif
#ifndef WIN32
// Since MSVC does not support the __attribute(constructor)__ extension, an alternative through
//   static object construction is implemented. 
//   See https://stackoverflow.com/questions/1113409/attribute-constructor-equivalent-in-vc for a similar
//   approach.
static void __attribute__((constructor)) tryload_libjack()
#else
static int tryload_libjack();
static int tryload_static = tryload_libjack();
static int tryload_libjack()
#endif
{
    if (getenv("SKIP_LIBJACK") == 0) { // just in case libjack is causing troubles..
    #ifdef __APPLE__
        libjack_handle = dlopen("libjack.0.dylib", RTLD_LAZY);
        if (!libjack_handle) {
            qDebug("dlopen error : %s ", dlerror());
        }
        libjack_handle = dlopen("/usr/local/lib/libjack.0.dylib", RTLD_LAZY);
        if (!libjack_handle) {
            qDebug("dlopen error : %s ", dlerror());
        }
    #elif defined(WIN32)
        // Force char implementation of library instead of possibly wchar_t implementation to be called.
        #ifdef _WIN64
            libjack_handle = LoadLibraryA("libjack64.dll");
        #else
            libjack_handle = LoadLibraryA("libjack.dll");
        #endif
    #else
        libjack_handle = dlopen("libjack.so.0", RTLD_LAZY);
    #endif
    }
    libjack_is_present = (libjack_handle != 0);
#ifdef WIN32
//#if  (defined (_MSCVER) || defined (_MSC_VER))
    return 1;
#endif
}

void *load_jack_function(const char *fn_name)
{
    void *fn = 0;
    if (!libjack_handle) {
        qDebug("libjack not found, so do not try to load  %s ffs  !", fn_name);
        return 0;
    }
#ifdef WIN32
    fn = (void*)GetProcAddress(libjack_handle, fn_name);
#else
    fn = dlsym(libjack_handle, fn_name);
#endif
    if (!fn) {
#ifdef WIN32
        char* lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0,NULL );
        qDebug("could not GetProcAddress( %s ), %s ", fn_name, lpMsgBuf) ;
#else
        qDebug ("could not dlsym( %s ), %s ", fn_name, dlerror()) ;
#endif
    }
    return fn;
}

#define DECL_FUNCTION(return_type, fn_name, arguments_types, arguments) \
  typedef return_type (*fn_name##_ptr_t)arguments_types;                \
  return_type fn_name arguments_types {                                 \
    static fn_name##_ptr_t fn = 0;                                      \
    if (fn == 0) { fn = (fn_name##_ptr_t)load_jack_function(#fn_name); } \
    if (fn) return (*fn)arguments;                                      \
    else return (return_type)-1;                                                      \
  }

#define DECL_FUNCTION_NULL(return_type, fn_name, arguments_types, arguments) \
  typedef return_type (*fn_name##_ptr_t)arguments_types;                \
  return_type fn_name arguments_types {                                 \
    static fn_name##_ptr_t fn = 0;                                      \
    if (fn == 0) { fn = (fn_name##_ptr_t)load_jack_function(#fn_name); } \
    if (fn) return (*fn)arguments;                                      \
    else return (return_type)0;                                                      \
  }

#define DECL_VOID_FUNCTION(fn_name, arguments_types, arguments)         \
  typedef void (*fn_name##_ptr_t)arguments_types;                       \
  void fn_name arguments_types {                                        \
    static fn_name##_ptr_t fn = 0;                                      \
    if (fn == 0) { fn = (fn_name##_ptr_t)load_jack_function(#fn_name); } \
    if (fn) (*fn)arguments;                                             \
  }


DECL_VOID_FUNCTION(jack_get_version, (int *major_ptr, int *minor_ptr, int *micro_ptr, int *proto_ptr), (major_ptr, minor_ptr, micro_ptr, proto_ptr));
DECL_FUNCTION_NULL(const char *, jack_get_version_string, (), ());
DECL_FUNCTION_NULL(jack_client_t *, jack_client_open, (const char *client_name, jack_options_t options, jack_status_t *status, ...),
              (client_name, options, status));
DECL_FUNCTION(int, jack_client_close, (jack_client_t *client), (client));
DECL_FUNCTION_NULL(jack_client_t *, jack_client_new, (const char *client_name), (client_name));
DECL_FUNCTION(int, jack_client_name_size, (), ());
DECL_FUNCTION_NULL(char*, jack_get_client_name, (jack_client_t *client), (client));
DECL_FUNCTION(int, jack_internal_client_new, (const char *client_name,
                                            const char *load_name,
                                            const char *load_init), (client_name, load_name, load_init));
DECL_VOID_FUNCTION(jack_internal_client_close, (const char *client_name), (client_name));
DECL_FUNCTION(int, jack_is_realtime, (jack_client_t *client), (client));
//WS DECL_VOID_FUNCTION(jack_on_shutdown, (jack_client_t *client, JackShutdownCallback shutdown_callback, void *arg), (client, shutdown_callback, arg));
//DECL_VOID_FUNCTION(jack_on_info_shutdown, (jack_client_t* client, JackInfoShutdownCallback shutdown_callback, void* arg), (client, shutdown_callback, arg));
DECL_FUNCTION(int, jack_set_process_callback, (jack_client_t *client,
                                            JackProcessCallback process_callback,
                                            void *arg), (client, process_callback, arg));
DECL_FUNCTION(jack_nframes_t, jack_thread_wait, (jack_client_t *client, int status), (client, status));

//
DECL_FUNCTION(jack_nframes_t, jack_cycle_wait, (jack_client_t *client), (client));
DECL_VOID_FUNCTION(jack_cycle_signal, (jack_client_t *client, int status), (client, status));
//DECL_FUNCTION(int, jack_set_process_thread, (jack_client_t *client,
//                                            JackThreadCallback fun,
//                                            void *arg), (client, fun, arg));
DECL_FUNCTION(int, jack_set_thread_init_callback, (jack_client_t *client,
                                            JackThreadInitCallback thread_init_callback,
                                            void *arg), (client, thread_init_callback, arg));
DECL_FUNCTION(int, jack_set_freewheel_callback, (jack_client_t *client,
                                            JackFreewheelCallback freewheel_callback,
                                            void *arg), (client, freewheel_callback, arg));
DECL_FUNCTION(int, jack_set_freewheel, (jack_client_t *client, int onoff), (client, onoff));
DECL_FUNCTION(int, jack_set_buffer_size, (jack_client_t *client, jack_nframes_t nframes), (client, nframes));
DECL_FUNCTION(int, jack_set_buffer_size_callback, (jack_client_t *client,
                                            JackBufferSizeCallback bufsize_callback,
                                            void *arg), (client, bufsize_callback, arg));
DECL_FUNCTION(int, jack_set_sample_rate_callback, (jack_client_t *client,
                                            JackSampleRateCallback srate_callback,
                                            void *arg), (client, srate_callback, arg));
DECL_FUNCTION(int, jack_set_client_registration_callback, (jack_client_t *client,
                                            JackClientRegistrationCallback registration_callback,
                                            void *arg), (client, registration_callback, arg));
DECL_FUNCTION(int, jack_set_port_registration_callback, (jack_client_t *client,
                                            JackPortRegistrationCallback registration_callback,
                                            void *arg), (client, registration_callback, arg));
DECL_FUNCTION(int, jack_set_port_connect_callback, (jack_client_t *client,
                                            JackPortConnectCallback connect_callback,
                                            void *arg), (client, connect_callback, arg));
//WS DECL_FUNCTION(int, jack_set_port_rename_callback, (jack_client_t *client,
//                                            JackPortRenameCallback rename_callback,
//                                            void *arg), (client, rename_callback, arg));
DECL_FUNCTION(int, jack_set_graph_order_callback, (jack_client_t *client,
                                            JackGraphOrderCallback graph_callback,
                                            void *arg), (client, graph_callback, arg));
DECL_FUNCTION(int, jack_set_xrun_callback, (jack_client_t *client,
                                            JackXRunCallback xrun_callback,
                                            void *arg), (client, xrun_callback, arg));
//DECL_FUNCTION(int, jack_set_latency_callback, (jack_client_t *client,
//                                            JackLatencyCallback latency_callback,
//                                            void *arg), (client, latency_callback, arg));
DECL_FUNCTION(int, jack_activate, (jack_client_t *client), (client));
DECL_FUNCTION(int, jack_deactivate, (jack_client_t *client), (client));
DECL_FUNCTION_NULL(jack_port_t *, jack_port_register, (jack_client_t *client, const char *port_name, const char *port_type,
                                                  unsigned long flags, unsigned long buffer_size),
              (client, port_name, port_type, flags, buffer_size));
DECL_FUNCTION(int, jack_port_unregister, (jack_client_t *client, jack_port_t* port), (client, port));
DECL_FUNCTION_NULL(void *, jack_port_get_buffer, (jack_port_t *port, jack_nframes_t nframes), (port, nframes));
DECL_FUNCTION_NULL(const char*, jack_port_name, (const jack_port_t *port), (port));
DECL_FUNCTION_NULL(const char*, jack_port_short_name, (const jack_port_t *port), (port));
DECL_FUNCTION(int, jack_port_flags, (const jack_port_t *port), (port));
DECL_FUNCTION_NULL(const char*, jack_port_type, (const jack_port_t *port), (port));
//WS DECL_FUNCTION(jack_port_type_id_t, jack_port_type_id, (const jack_port_t *port), (port));
DECL_FUNCTION(int, jack_port_is_mine, (const jack_client_t *client, const jack_port_t* port), (client, port));
DECL_FUNCTION(int, jack_port_connected, (const jack_port_t *port), (port));
DECL_FUNCTION(int, jack_port_connected_to, (const jack_port_t *port, const char *port_name), (port, port_name));
DECL_FUNCTION_NULL(const char**, jack_port_get_connections, (const jack_port_t *port), (port));
DECL_FUNCTION_NULL(const char**, jack_port_get_all_connections, (const jack_client_t *client,const jack_port_t *port), (client, port));
DECL_FUNCTION(int, jack_port_tie, (jack_port_t *src, jack_port_t *dst), (src, dst));
DECL_FUNCTION(int, jack_port_untie, (jack_port_t *port), (port));
DECL_FUNCTION(jack_nframes_t, jack_port_get_latency, (jack_port_t *port), (port));
DECL_FUNCTION(jack_nframes_t, jack_port_get_total_latency ,(jack_client_t * client, jack_port_t *port), (client, port));
DECL_VOID_FUNCTION(jack_port_set_latency, (jack_port_t * port, jack_nframes_t frames), (port, frames));
DECL_FUNCTION(int, jack_recompute_total_latency, (jack_client_t* client, jack_port_t* port), (client, port));
//DECL_VOID_FUNCTION(jack_port_get_latency_range, (jack_port_t *port, jack_latency_callback_mode_t mode, jack_latency_range_t *range), (port, mode, range));
//DECL_VOID_FUNCTION(jack_port_set_latency_range, (jack_port_t *port, jack_latency_callback_mode_t mode, jack_latency_range_t *range), (port, mode, range));
DECL_FUNCTION(int, jack_recompute_total_latencies, (jack_client_t* client),(client));

DECL_FUNCTION(int, jack_port_set_name, (jack_port_t *port, const char *port_name), (port, port_name));
//DECL_FUNCTION(int, jack_port_rename, (jack_client_t *client, jack_port_t *port, const char *port_name), (client, port, port_name));
DECL_FUNCTION(int, jack_port_set_alias, (jack_port_t *port, const char *alias), (port, alias));
DECL_FUNCTION(int, jack_port_unset_alias, (jack_port_t *port, const char *alias), (port, alias));
DECL_FUNCTION(int, jack_port_get_aliases, (const jack_port_t *port, char* const aliases[2]), (port,aliases));
DECL_FUNCTION(int, jack_port_request_monitor, (jack_port_t *port, int onoff), (port, onoff));
DECL_FUNCTION(int, jack_port_request_monitor_by_name, (jack_client_t *client, const char *port_name, int onoff), (client, port_name, onoff));
DECL_FUNCTION(int, jack_port_ensure_monitor, (jack_port_t *port, int onoff), (port, onoff));
DECL_FUNCTION(int, jack_port_monitoring_input, (jack_port_t *port) ,(port));
DECL_FUNCTION(int, jack_connect, (jack_client_t * client, const char *source_port, const char *destination_port), (client, source_port, destination_port));
DECL_FUNCTION(int, jack_disconnect, (jack_client_t * client, const char *source_port, const char *destination_port), (client, source_port, destination_port));
DECL_FUNCTION(int, jack_port_disconnect, (jack_client_t * client, jack_port_t * port), (client, port));
DECL_FUNCTION(int, jack_port_name_size,(),());
DECL_FUNCTION(int, jack_port_type_size,(),());
//DECL_FUNCTION(size_t, jack_port_type_get_buffer_size, (jack_client_t *client, const char* port_type), (client, port_type));

DECL_FUNCTION(jack_nframes_t, jack_get_sample_rate, (jack_client_t *client), (client));
DECL_FUNCTION(jack_nframes_t, jack_get_buffer_size, (jack_client_t *client), (client));
DECL_FUNCTION_NULL(const char**, jack_get_ports, (jack_client_t *client, const char *port_name_pattern, const char * type_name_pattern,
                                             unsigned long flags), (client, port_name_pattern, type_name_pattern, flags));
DECL_FUNCTION_NULL(jack_port_t *, jack_port_by_name, (jack_client_t * client, const char *port_name), (client, port_name));
DECL_FUNCTION_NULL(jack_port_t *, jack_port_by_id, (jack_client_t *client, jack_port_id_t port_id), (client, port_id));

DECL_FUNCTION(int, jack_engine_takeover_timebase, (jack_client_t * client), (client));
DECL_FUNCTION(jack_nframes_t, jack_frames_since_cycle_start, (const jack_client_t * client), (client));
DECL_FUNCTION(jack_time_t, jack_get_time, (), ());
DECL_FUNCTION(jack_nframes_t, jack_time_to_frames, (const jack_client_t *client, jack_time_t time), (client, time));
DECL_FUNCTION(jack_time_t, jack_frames_to_time, (const jack_client_t *client, jack_nframes_t frames), (client, frames));
DECL_FUNCTION(jack_nframes_t, jack_frame_time, (const jack_client_t *client), (client));
DECL_FUNCTION(jack_nframes_t, jack_last_frame_time, (const jack_client_t *client), (client));
DECL_FUNCTION(float, jack_cpu_load, (jack_client_t *client), (client));
DECL_FUNCTION_NULL(jack_native_thread_t, jack_client_thread_id, (jack_client_t *client), (client));
DECL_VOID_FUNCTION(jack_set_error_function, (print_function fun), (fun));
DECL_VOID_FUNCTION(jack_set_info_function, (print_function fun), (fun));

DECL_FUNCTION(float, jack_get_max_delayed_usecs, (jack_client_t *client), (client));
DECL_FUNCTION(float, jack_get_xrun_delayed_usecs, (jack_client_t *client), (client));
DECL_VOID_FUNCTION(jack_reset_max_delayed_usecs, (jack_client_t *client), (client));

DECL_FUNCTION(int, jack_release_timebase, (jack_client_t *client), (client));
DECL_FUNCTION(int, jack_set_sync_callback, (jack_client_t *client, JackSyncCallback sync_callback, void *arg), (client, sync_callback, arg));
DECL_FUNCTION(int, jack_set_sync_timeout, (jack_client_t *client, jack_time_t timeout), (client, timeout));
DECL_FUNCTION(int, jack_set_timebase_callback, (jack_client_t *client,
                                                int conditional,
                                                JackTimebaseCallback timebase_callback,
                                                void *arg), (client, conditional, timebase_callback, arg));
DECL_FUNCTION(int, jack_transport_locate, (jack_client_t *client, jack_nframes_t frame), (client, frame));
DECL_FUNCTION(jack_transport_state_t, jack_transport_query, (const jack_client_t *client, jack_position_t *pos), (client, pos));
DECL_FUNCTION(jack_nframes_t, jack_get_current_transport_frame, (const jack_client_t *client), (client));
DECL_FUNCTION(int, jack_transport_reposition, (jack_client_t *client, const jack_position_t *pos), (client, pos));
DECL_VOID_FUNCTION(jack_transport_start, (jack_client_t *client), (client));
DECL_VOID_FUNCTION(jack_transport_stop, (jack_client_t *client), (client));
//DECL_VOID_FUNCTION(jack_get_transport_info, (jack_client_t *client, jack_transport_info_t *tinfo), (client,tinfo));
//DECL_VOID_FUNCTION(jack_set_transport_info, (jack_client_t *client, jack_transport_info_t *tinfo), (client,tinfo));

DECL_FUNCTION(int, jack_client_real_time_priority, (jack_client_t* client), (client));
DECL_FUNCTION(int, jack_client_max_real_time_priority, (jack_client_t* client), (client));
DECL_FUNCTION(int, jack_acquire_real_time_scheduling, (jack_native_thread_t thread, int priority), (thread, priority));
DECL_FUNCTION(int, jack_client_create_thread, (jack_client_t* client,
                                      jack_native_thread_t *thread,
                                      int priority,
                                      int realtime, 	// boolean
                                      thread_routine routine,
                                      void *arg), (client, thread, priority, realtime, routine, arg));
DECL_FUNCTION(int, jack_drop_real_time_scheduling, (jack_native_thread_t thread), (thread));

DECL_FUNCTION(int, jack_client_stop_thread, (jack_client_t* client, jack_native_thread_t thread), (client, thread));
DECL_FUNCTION(int, jack_client_kill_thread, (jack_client_t* client, jack_native_thread_t thread), (client, thread));
#ifndef WIN32
//WS DECL_VOID_FUNCTION(jack_set_thread_creator, (jack_thread_creator_t jtc), (jtc));
#endif
DECL_FUNCTION(char *, jack_get_internal_client_name, (jack_client_t *client, jack_intclient_t intclient), (client, intclient));
DECL_FUNCTION(jack_intclient_t, jack_internal_client_handle, (jack_client_t *client, const char *client_name, jack_status_t *status), (client, client_name, status));
/*
DECL_FUNCTION(jack_intclient_t, jack_internal_client_load, (jack_client_t *client,
                                                            const char *client_name,
                                                            jack_options_t options,
                                                            jack_status_t *status
                                                            , ...), (client, client_name, options, status, ...));
*/
DECL_FUNCTION(jack_status_t, jack_internal_client_unload, (jack_client_t *client, jack_intclient_t intclient), (client, intclient));
DECL_VOID_FUNCTION(jack_free, (void* ptr), (ptr));

// session
//DECL_FUNCTION(int, jack_set_session_callback, (jack_client_t* ext_client, JackSessionCallback session_callback, void* arg), (ext_client, session_callback, arg));
//DECL_FUNCTION(jack_session_command_t*, jack_session_notify, (jack_client_t* ext_client, const char* target, jack_session_event_type_t ev_type, const char* path), (ext_client, target, ev_type, path));
//DECL_FUNCTION(int, jack_session_reply, (jack_client_t* ext_client, jack_session_event_t *event), (ext_client, event));
//DECL_VOID_FUNCTION(jack_session_event_free, (jack_session_event_t* ev), (ev));
//DECL_FUNCTION(char*, jack_client_get_uuid, (jack_client_t* ext_client),(ext_client));
//DECL_FUNCTION(char*, jack_get_uuid_for_client_name, (jack_client_t* ext_client, const char* client_name),(ext_client, client_name));
//DECL_FUNCTION(char*, jack_get_client_name_by_uuid, (jack_client_t* ext_client, const char* client_uuid),(ext_client, client_uuid));
//DECL_FUNCTION(int, jack_reserve_client_name, (jack_client_t* ext_client, const char* name, const char* uuid),(ext_client, name, uuid));
//DECL_VOID_FUNCTION(jack_session_commands_free, (jack_session_command_t *cmds),(cmds));
//DECL_FUNCTION(int, jack_client_has_session_callback, (jack_client_t *client, const char* client_name),(client, client_name));

// MIDI

DECL_FUNCTION(jack_nframes_t, jack_midi_get_event_count, (void* port_buffer), (port_buffer));
DECL_FUNCTION(int, jack_midi_event_get, (jack_midi_event_t* event, void* port_buffer, jack_nframes_t event_index), (event, port_buffer, event_index)) ;
DECL_VOID_FUNCTION(jack_midi_clear_buffer, (void* port_buffer), (port_buffer));
DECL_FUNCTION(size_t, jack_midi_max_event_size, (void* port_buffer), (port_buffer));
DECL_FUNCTION_NULL(jack_midi_data_t*, jack_midi_event_reserve, (void* port_buffer, jack_nframes_t time, size_t data_size), (port_buffer, time, data_size));
DECL_FUNCTION(int, jack_midi_event_write, (void* port_buffer, jack_nframes_t time, const jack_midi_data_t* data, size_t data_size), (port_buffer, time, data, data_size));
DECL_FUNCTION(jack_nframes_t, jack_midi_get_lost_event_count, (void* port_buffer), (port_buffer));
