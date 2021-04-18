// Copyright 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/synchronization/lock_impl.h"

#include <errno.h>

#include "base/logging.h"

namespace base {
namespace internal {

LockImpl::LockImpl() {
#ifndef NDEBUG
  // In debug, setup attributes for lock error checking.
  pthread_mutexattr_t mta;
  int rv = pthread_mutexattr_init(&mta);
  DCHECK_EQ(rv, 0);
  rv = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
  DCHECK_EQ(rv, 0);
  rv = pthread_mutex_init(&native_handle_, &mta);
  DCHECK_EQ(rv, 0);
  rv = pthread_mutexattr_destroy(&mta);
  DCHECK_EQ(rv, 0);
#else
  // In release, go with the default lock attributes.
  pthread_mutex_init(&native_handle_, NULL);
#endif
}

LockImpl::~LockImpl() {
  int rv = pthread_mutex_destroy(&native_handle_);
  DCHECK_EQ(rv, 0);
}

bool LockImpl::Try() {
  int rv = pthread_mutex_trylock(&native_handle_);
  DCHECK(rv == 0 || rv == EBUSY);
  return rv == 0;
}

void LockImpl::Lock() {
  int rv = pthread_mutex_lock(&native_handle_);
  DCHECK_EQ(rv, 0);
}

void LockImpl::Unlock() {
  int rv = pthread_mutex_unlock(&native_handle_);
  DCHECK_EQ(rv, 0);
}

}  // namespace internal
}  // namespace base
