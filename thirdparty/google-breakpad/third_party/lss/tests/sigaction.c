/* Copyright 2020, Google Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "test_skel.h"

void test_handler(int sig) {}

int main(int argc, char *argv[]) {
  const size_t kSigsetSize = sizeof(struct kernel_sigset_t);
  struct kernel_sigaction action = {};
  // Invalid signal returns EINVAL.
  assert(sys_rt_sigaction(SIGKILL, &action, NULL, kSigsetSize) == -1);
  assert(errno == EINVAL);

  // Set an action.
  action.sa_handler_ = test_handler;
  action.sa_flags = SA_SIGINFO;
  assert(sys_sigemptyset(&action.sa_mask) == 0);
  assert(sys_sigaddset(&action.sa_mask, SIGPIPE) == 0);
  assert(sys_rt_sigaction(SIGSEGV, &action, NULL, kSigsetSize) == 0);

  // Retrieve the action.
  struct kernel_sigaction old_action = {};
  assert(sys_rt_sigaction(SIGSEGV, NULL, &old_action, kSigsetSize) == 0);
  assert(memcmp(&action, &old_action, sizeof(action)) == 0);

  return 0;
}
