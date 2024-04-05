/*-*- Mode: C; c-basic-offset: 8 -*-*/

#ifndef foortkithfoo
#define foortkithfoo

/***
  Copyright 2009 Lennart Poettering
  Copyright 2010 David Henningsson <diwic@ubuntu.com>

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

#ifdef DBUS_SUPPORT

#include <sys/types.h>
#include <dbus/dbus.h>

#ifdef __cplusplus
extern "C" {
#endif

/* This is the reference implementation for a client for
 * RealtimeKit. You don't have to use this, but if do, just copy these
 * sources into your repository */

#define RTKIT_SERVICE_NAME "org.freedesktop.RealtimeKit1"
#define RTKIT_OBJECT_PATH "/org/freedesktop/RealtimeKit1"

/* This is mostly equivalent to sched_setparam(thread, SCHED_RR, {
 * .sched_priority = priority }). 'thread' needs to be a kernel thread
 * id as returned by gettid(), not a pthread_t! If 'thread' is 0 the
 * current thread is used. The returned value is a negative errno
 * style error code, or 0 on success. */
int fluid_rtkit_make_realtime(pid_t thread, int priority);

#ifdef __cplusplus
}
#endif

#endif

#endif
