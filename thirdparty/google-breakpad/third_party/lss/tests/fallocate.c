/* Copyright 2019, Google Inc.  All rights reserved.
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

int main(int argc, char *argv[]) {
  int fd = 0, mode = 0;
  loff_t offset = 0, len = 0;

  // Bad file descriptor.
  fd = -1;
  assert(sys_fallocate(fd, mode, offset, len) == -1);
  assert(errno == EBADF);

  char filename[] = "tempfile.XXXXXX";
  fd = mkstemp(filename);
  assert(fd >= 0);

  // Invalid len.
  assert(sys_fallocate(fd, mode, offset, len) == -1);
  assert(errno == EINVAL);

  // Small offset and length succeeds.
  len = 4096;
  assert(sys_fallocate(fd, mode, offset, len) == 0);

  // Large offset succeeds and isn't truncated.
  offset = 1llu + UINT32_MAX;
  assert(sys_fallocate(fd, mode , offset, len) == 0);

#if defined(__NR_fstat64)
  struct kernel_stat64 st;
  assert(sys_fstat64(fd, &st) == 0);
#else
  struct kernel_stat st;
  assert(sys_fstat(fd, &st) == 0);
#endif
  assert(st.st_size == offset + len);

  return 0;
}
