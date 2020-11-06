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

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
  char buffer[BUFFER_SIZE];
  // Zero it out so we can check later that it's at least not all 0s.
  memset(buffer, 0, BUFFER_SIZE);
  bool buffer_contains_all_zeros = true;

  // Don't bother passing any flags. (If we're using lss, we might not have the
  // right header files with the flags defined anyway, and we'd have to copy
  // this in here too, and risk getting out of sync in yet another way.)
  const ssize_t r = sys_getrandom(buffer, BUFFER_SIZE, 0);

  // Make sure it either worked, or that it's just not supported.
  assert(r == BUFFER_SIZE || errno == ENOSYS);

  if (r == BUFFER_SIZE) {
    // If all the bytes are 0, it didn't really work.
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
      if (buffer[i] != 0) {
        buffer_contains_all_zeros = false;
      }
    }
    assert(!buffer_contains_all_zeros);
  }

  return 0;
}
