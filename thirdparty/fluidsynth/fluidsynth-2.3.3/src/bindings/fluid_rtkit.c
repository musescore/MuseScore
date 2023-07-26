/*-*- Mode: C; c-basic-offset: 8 -*-*/

/***
  Copyright 2009 Lennart Poettering
  Copyright 2010 David Henningsson

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation files
  (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
  ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
***/

#ifndef _GNU_SOURCE
// required for syscall()
#define _GNU_SOURCE
#endif

#include "fluid_sys.h"

#ifdef DBUS_SUPPORT

#include "fluid_rtkit.h"


#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__DragonFly__)

#include <sys/syscall.h>
#include <sys/resource.h>

#if defined(__FreeBSD__) || defined(__DragonFly__)
#include <pthread_np.h>
#endif

static pid_t _gettid(void)
{
#if defined(__FreeBSD__) || defined(__DragonFly__)
    return pthread_getthreadid_np();
#else
    return (pid_t) syscall(SYS_gettid);
#endif
}

static int translate_error(const char *name)
{
    if(FLUID_STRCMP(name, DBUS_ERROR_NO_MEMORY) == 0)
    {
        return -ENOMEM;
    }

    if(FLUID_STRCMP(name, DBUS_ERROR_SERVICE_UNKNOWN) == 0 ||
            FLUID_STRCMP(name, DBUS_ERROR_NAME_HAS_NO_OWNER) == 0)
    {
        return -ENOENT;
    }

    if(FLUID_STRCMP(name, DBUS_ERROR_ACCESS_DENIED) == 0 ||
            FLUID_STRCMP(name, DBUS_ERROR_AUTH_FAILED) == 0)
    {
        return -EACCES;
    }

    return -EIO;
}

static long long rtkit_get_int_property(DBusConnection *connection, const char *propname, long long *propval)
{
    DBusMessage *m = NULL, *r = NULL;
    DBusMessageIter iter, subiter;
    dbus_int64_t i64;
    dbus_int32_t i32;
    DBusError error;
    int current_type;
    long long ret;
    const char *interfacestr = "org.freedesktop.RealtimeKit1";

    dbus_error_init(&error);

    if(!(m = dbus_message_new_method_call(
                 RTKIT_SERVICE_NAME,
                 RTKIT_OBJECT_PATH,
                 "org.freedesktop.DBus.Properties",
                 "Get")))
    {
        ret = -ENOMEM;
        goto finish;
    }

    if(!dbus_message_append_args(
                m,
                DBUS_TYPE_STRING, &interfacestr,
                DBUS_TYPE_STRING, &propname,
                DBUS_TYPE_INVALID))
    {
        ret = -ENOMEM;
        goto finish;
    }

    if(!(r = dbus_connection_send_with_reply_and_block(connection, m, -1, &error)))
    {
        ret = translate_error(error.name);
        goto finish;
    }

    if(dbus_set_error_from_message(&error, r))
    {
        ret = translate_error(error.name);
        goto finish;
    }

    ret = -EBADMSG;
    dbus_message_iter_init(r, &iter);

    while((current_type = dbus_message_iter_get_arg_type(&iter)) != DBUS_TYPE_INVALID)
    {

        if(current_type == DBUS_TYPE_VARIANT)
        {
            dbus_message_iter_recurse(&iter, &subiter);

            while((current_type = dbus_message_iter_get_arg_type(&subiter)) != DBUS_TYPE_INVALID)
            {

                if(current_type == DBUS_TYPE_INT32)
                {
                    dbus_message_iter_get_basic(&subiter, &i32);
                    *propval = i32;
                    ret = 0;
                }

                if(current_type == DBUS_TYPE_INT64)
                {
                    dbus_message_iter_get_basic(&subiter, &i64);
                    *propval = i64;
                    ret = 0;
                }

                dbus_message_iter_next(&subiter);
            }
        }

        dbus_message_iter_next(&iter);
    }

finish:

    if(m)
    {
        dbus_message_unref(m);
    }

    if(r)
    {
        dbus_message_unref(r);
    }

    dbus_error_free(&error);

    return ret;
}

int rtkit_get_max_realtime_priority(DBusConnection *connection)
{
    long long retval = 0;
    int err;

    err = rtkit_get_int_property(connection, "MaxRealtimePriority", &retval);
    return err < 0 ? err : retval;
}

int rtkit_get_min_nice_level(DBusConnection *connection, int *min_nice_level)
{
    long long retval = 0;
    int err;

    err = rtkit_get_int_property(connection, "MinNiceLevel", &retval);

    if(err >= 0)
    {
        *min_nice_level = retval;
    }

    return err;
}

long long rtkit_get_rttime_nsec_max(DBusConnection *connection)
{
    long long retval = 0;
    int err;

    err = rtkit_get_int_property(connection, "RTTimeNSecMax", &retval);
    return err < 0 ? err : retval;
}

int rtkit_make_realtime(DBusConnection *connection, pid_t thread, int priority)
{
    DBusMessage *m = NULL, *r = NULL;
    dbus_uint64_t u64;
    dbus_uint32_t u32;
    DBusError error;
    int ret;


    dbus_error_init(&error);

    if(thread == 0)
    {
        thread = _gettid();
    }

    if(!(m = dbus_message_new_method_call(
                 RTKIT_SERVICE_NAME,
                 RTKIT_OBJECT_PATH,
                 "org.freedesktop.RealtimeKit1",
                 "MakeThreadRealtime")))
    {
        ret = -ENOMEM;
        goto finish;
    }

    u64 = (dbus_uint64_t) thread;
    u32 = (dbus_uint32_t) priority;

    if(!dbus_message_append_args(
                m,
                DBUS_TYPE_UINT64, &u64,
                DBUS_TYPE_UINT32, &u32,
                DBUS_TYPE_INVALID))
    {
        ret = -ENOMEM;
        goto finish;
    }

    if(!(r = dbus_connection_send_with_reply_and_block(connection, m, -1, &error)))
    {
        ret = translate_error(error.name);
        goto finish;
    }


    if(dbus_set_error_from_message(&error, r))
    {
        ret = translate_error(error.name);
        goto finish;
    }

    ret = 0;

finish:

    if(m)
    {
        dbus_message_unref(m);
    }

    if(r)
    {
        dbus_message_unref(r);
    }

    dbus_error_free(&error);

    return ret;
}

int rtkit_make_high_priority(DBusConnection *connection, pid_t thread, int nice_level)
{
    DBusMessage *m = NULL, *r = NULL;
    dbus_uint64_t u64;
    dbus_int32_t s32;
    DBusError error;
    int ret;

    dbus_error_init(&error);

    if(thread == 0)
    {
        thread = _gettid();
    }

    if(!(m = dbus_message_new_method_call(
                 RTKIT_SERVICE_NAME,
                 RTKIT_OBJECT_PATH,
                 "org.freedesktop.RealtimeKit1",
                 "MakeThreadHighPriority")))
    {
        ret = -ENOMEM;
        goto finish;
    }

    u64 = (dbus_uint64_t) thread;
    s32 = (dbus_int32_t) nice_level;

    if(!dbus_message_append_args(
                m,
                DBUS_TYPE_UINT64, &u64,
                DBUS_TYPE_INT32, &s32,
                DBUS_TYPE_INVALID))
    {
        ret = -ENOMEM;
        goto finish;
    }



    if(!(r = dbus_connection_send_with_reply_and_block(connection, m, -1, &error)))
    {
        ret = translate_error(error.name);
        goto finish;
    }


    if(dbus_set_error_from_message(&error, r))
    {
        ret = translate_error(error.name);
        goto finish;
    }

    ret = 0;

finish:

    if(m)
    {
        dbus_message_unref(m);
    }

    if(r)
    {
        dbus_message_unref(r);
    }

    dbus_error_free(&error);

    return ret;
}

#ifndef RLIMIT_RTTIME
#  define RLIMIT_RTTIME 15
#endif

#define MAKE_REALTIME_RETURN(_value) \
  do { \
    dbus_connection_close(conn); \
    dbus_connection_unref(conn); \
    return _value; \
  } while (0)


int fluid_rtkit_make_realtime(pid_t thread, int priority)
{
    DBusConnection *conn = NULL;
    DBusError error;
    int max_prio, res;
    long long max_rttime;
    struct rlimit old_limit, new_limit;

    if(!dbus_threads_init_default())
    {
        return -ENOMEM;
    }

    /* Initialize system bus connection */
    dbus_error_init(&error);
    conn = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);

    if(conn == NULL)
    {
        res = translate_error(error.name);
        dbus_error_free(&error);
        return res;
    }

    dbus_error_free(&error);

    /* Make sure we don't fail by wanting too much */
    max_prio = rtkit_get_max_realtime_priority(conn);

    if(max_prio < 0)
    {
        MAKE_REALTIME_RETURN(max_prio);
    }

    if(priority >= max_prio)
    {
        priority = max_prio;
    }

    /* Enforce RLIMIT_RTTIME, also a must for obtaining rt prio through rtkit */
    max_rttime = rtkit_get_rttime_nsec_max(conn);

    if(max_rttime < 0)
    {
        MAKE_REALTIME_RETURN(max_rttime);
    }

    new_limit.rlim_cur = new_limit.rlim_max = max_rttime;

    if(getrlimit(RLIMIT_RTTIME, &old_limit) < 0)
    {
        MAKE_REALTIME_RETURN(-1);
    }

    if(setrlimit(RLIMIT_RTTIME, &new_limit) < 0)
    {
        MAKE_REALTIME_RETURN(-1);
    }

    /* Finally, let's try */
    res = rtkit_make_realtime(conn, thread, priority);

    if(res != 0)
    {
        setrlimit(RLIMIT_RTTIME, &old_limit);
    }

    MAKE_REALTIME_RETURN(res);

}


#else

int rtkit_make_realtime(DBusConnection *connection, pid_t thread, int priority)
{
    return -ENOTSUP;
}

int rtkit_make_high_priority(DBusConnection *connection, pid_t thread, int nice_level)
{
    return -ENOTSUP;
}

int rtkit_get_max_realtime_priority(DBusConnection *connection)
{
    return -ENOTSUP;
}

int rtkit_get_min_nice_level(DBusConnection *connection, int *min_nice_level)
{
    return -ENOTSUP;
}

long long rtkit_get_rttime_nsec_max(DBusConnection *connection)
{
    return -ENOTSUP;
}

int fluid_rtkit_make_realtime(pid_t thread, int priority)
{
    return -ENOTSUP;
}

#endif /* defined(__linux__) || defined(__APPLE__) */

#endif /* DBUS_SUPPORT */
