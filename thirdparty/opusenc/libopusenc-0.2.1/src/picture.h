/* Copyright (C)2007-2013 Xiph.Org Foundation
   File: picture.h

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PICTURE_H
#define PICTURE_H

#include <opus.h>
#include "opusenc.h"

typedef enum{
  PIC_FORMAT_JPEG,
  PIC_FORMAT_PNG,
  PIC_FORMAT_GIF
}picture_format;

#define BASE64_LENGTH(len) (((len)+2)/3*4)

char *opeint_parse_picture_specification(const char *filename, int picture_type, const char *description,
                                  int *error, int *seen_file_icons);

char *opeint_parse_picture_specification_from_memory(const char *mem, size_t size, int picture_type, const char *description,
                                  int *error, int *seen_file_icons);

#define WRITE_U32_BE(buf, val) \
  do{ \
    (buf)[0]=(unsigned char)((val)>>24); \
    (buf)[1]=(unsigned char)((val)>>16); \
    (buf)[2]=(unsigned char)((val)>>8); \
    (buf)[3]=(unsigned char)(val); \
  } \
  while(0);

#endif /* PICTURE_H */
