/* Copyright (C)2012 Xiph.Org Foundation
   File: opus_header.c

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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "opus_header.h"
#include <string.h>
#include <stdio.h>

/* Header contents:
  - "OpusHead" (64 bits)
  - version number (8 bits)
  - Channels C (8 bits)
  - Pre-skip (16 bits)
  - Sampling rate (32 bits)
  - Gain in dB (16 bits, S7.8)
  - Mapping (8 bits, 0=single stream (mono/stereo) 1=Vorbis mapping,
             2=ambisonics, 3=projection ambisonics, 4..239: reserved,
             240..254: experiments, 255: multistream with no mapping)

  - if (mapping != 0)
     - N = total number of streams (8 bits)
     - M = number of paired streams (8 bits)
     - if (mapping != a projection family)
       - C times channel origin
          - if (C<2*M)
             - stream = byte/2
             - if (byte&0x1 == 0)
                 - left
               else
                 - right
          - else
             - stream = byte-M
    - else
       - D demixing matrix (C*(N+M)*16 bits)
*/

typedef struct {
   unsigned char *data;
   int maxlen;
   int pos;
} Packet;

static int write_uint32(Packet *p, opus_uint32 val)
{
   if (p->pos>p->maxlen-4)
      return 0;
   p->data[p->pos  ] = (val    ) & 0xFF;
   p->data[p->pos+1] = (val>> 8) & 0xFF;
   p->data[p->pos+2] = (val>>16) & 0xFF;
   p->data[p->pos+3] = (val>>24) & 0xFF;
   p->pos += 4;
   return 1;
}

static int write_uint16(Packet *p, opus_uint16 val)
{
   if (p->pos>p->maxlen-2)
      return 0;
   p->data[p->pos  ] = (val    ) & 0xFF;
   p->data[p->pos+1] = (val>> 8) & 0xFF;
   p->pos += 2;
   return 1;
}

static int write_chars(Packet *p, const unsigned char *str, int nb_chars)
{
   int i;
   if (p->pos>p->maxlen-nb_chars)
      return 0;
   for (i=0;i<nb_chars;i++)
      p->data[p->pos++] = str[i];
   return 1;
}

static int write_matrix_chars(Packet *p, const OpusGenericEncoder *st)
{
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
  opus_int32 size;
  int ret;
  ret=opeint_encoder_ctl(st, OPUS_PROJECTION_GET_DEMIXING_MATRIX_SIZE(&size));
  if (ret != OPUS_OK) return 0;
  if (size>p->maxlen-p->pos) return 0;
  ret=opeint_encoder_ctl(st, OPUS_PROJECTION_GET_DEMIXING_MATRIX(&p->data[p->pos], size));
  if (ret != OPUS_OK) return 0;
  p->pos += size;
  return 1;
#else
  (void)p;
  (void)st;
  return 0;
#endif
}

int opeint_opus_header_get_size(const OpusHeader *h)
{
  int len=0;
  if (opeint_use_projection(h->channel_mapping))
  {
    /* 19 bytes from fixed header,
     * 2 bytes for nb_streams & nb_coupled,
     * 2 bytes per cell of demixing matrix, where:
     *    rows=channels, cols=nb_streams+nb_coupled
     */
    len=21+(h->channels*(h->nb_streams+h->nb_coupled)*2);
  }
  else
  {
    /* 19 bytes from fixed header,
     * 2 bytes for nb_streams & nb_coupled,
     * 1 byte per channel
     */
    len=21+h->channels;
  }
  return len;
}

int opeint_opus_header_to_packet(const OpusHeader *h, unsigned char *packet, int len, const OpusGenericEncoder *st)
{
   int i;
   Packet p;
   unsigned char ch;

   p.data = packet;
   p.maxlen = len;
   p.pos = 0;
   if (len<19)return 0;
   if (!write_chars(&p, (const unsigned char*)"OpusHead", 8))
      return 0;
   /* Version is 1 */
   ch = 1;
   if (!write_chars(&p, &ch, 1))
      return 0;

   ch = h->channels;
   if (!write_chars(&p, &ch, 1))
      return 0;

   if (!write_uint16(&p, h->preskip))
      return 0;

   if (!write_uint32(&p, h->input_sample_rate))
      return 0;

   if (opeint_use_projection(h->channel_mapping))
   {
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
      opus_int32 matrix_gain;
      int ret;
      ret=opeint_encoder_ctl(st, OPUS_PROJECTION_GET_DEMIXING_MATRIX_GAIN(&matrix_gain));
      if (ret != OPUS_OK) return 0;
      if (!write_uint16(&p, h->gain + matrix_gain))
         return 0;
#else
      return 0;
#endif
   }
   else
   {
      if (!write_uint16(&p, h->gain))
         return 0;
   }

   ch = h->channel_mapping;
   if (!write_chars(&p, &ch, 1))
      return 0;

   if (h->channel_mapping != 0)
   {
      ch = h->nb_streams;
      if (!write_chars(&p, &ch, 1))
         return 0;

      ch = h->nb_coupled;
      if (!write_chars(&p, &ch, 1))
         return 0;

      /* Multi-stream support */
      if (opeint_use_projection(h->channel_mapping))
      {
        if (!write_matrix_chars(&p, st))
           return 0;
      }
      else
      {
        for (i=0;i<h->channels;i++)
        {
          if (!write_chars(&p, &h->stream_map[i], 1))
             return 0;
        }
      }
   }

   return p.pos;
}

/*
 Comments will be stored in the Vorbis style.
 It is described in the "Structure" section of
    http://www.xiph.org/ogg/vorbis/doc/v-comment.html

 However, Opus and other non-vorbis formats omit the "framing_bit".

The comment header is decoded as follows:
  1) [vendor_length] = read an unsigned integer of 32 bits
  2) [vendor_string] = read a UTF-8 vector as [vendor_length] octets
  3) [user_comment_list_length] = read an unsigned integer of 32 bits
  4) iterate [user_comment_list_length] times {
     5) [length] = read an unsigned integer of 32 bits
     6) this iteration's user comment = read a UTF-8 vector as [length] octets
     }
  7) done.
*/

#define readint(buf, base) (((buf[base+3]<<24)&0xff000000)| \
                           ((buf[base+2]<<16)&0xff0000)| \
                           ((buf[base+1]<<8)&0xff00)| \
                           (buf[base]&0xff))
#define writeint(buf, base, val) do{ buf[base+3]=((val)>>24)&0xff; \
                                     buf[base+2]=((val)>>16)&0xff; \
                                     buf[base+1]=((val)>>8)&0xff; \
                                     buf[base]=(val)&0xff; \
                                 }while(0)

void opeint_comment_init(char **comments, int* length, const char *vendor_string)
{
  /*The 'vendor' field should be the actual encoding library used.*/
  int vendor_length=strlen(vendor_string);
  int user_comment_list_length=0;
  int len=8+4+vendor_length+4;
  char *p=(char*)malloc(len);
  if (p == NULL) {
    len=0;
  } else {
    memcpy(p, "OpusTags", 8);
    writeint(p, 8, vendor_length);
    memcpy(p+12, vendor_string, vendor_length);
    writeint(p, 12+vendor_length, user_comment_list_length);
  }
  *length=len;
  *comments=p;
}

int opeint_comment_add(char **comments, int* length, const char *tag, const char *val)
{
  char* p=*comments;
  int vendor_length=readint(p, 8);
  int user_comment_list_length=readint(p, 8+4+vendor_length);
  int tag_len=(tag?strlen(tag)+1:0);
  int val_len=strlen(val);
  int len=(*length)+4+tag_len+val_len;

  p=(char*)realloc(p, len);
  if (p == NULL) return 1;

  writeint(p, *length, tag_len+val_len);      /* length of comment */
  if(tag){
    memcpy(p+*length+4, tag, tag_len);        /* comment tag */
    (p+*length+4)[tag_len-1] = '=';           /* separator */
  }
  memcpy(p+*length+4+tag_len, val, val_len);  /* comment */
  writeint(p, 8+4+vendor_length, user_comment_list_length+1);
  *comments=p;
  *length=len;
  return 0;
}

void opeint_comment_pad(char **comments, int* length, int amount)
{
  if(amount>0){
    int i;
    int newlen;
    char* p=*comments;
    /*Make sure there is at least amount worth of padding free, and
       round up to the maximum that fits in the current ogg segments.*/
    newlen=(*length+amount+255)/255*255-1;
    p=realloc(p,newlen);
    if (p == NULL) return;
    for(i=*length;i<newlen;i++)p[i]=0;
    *comments=p;
    *length=newlen;
  }
}

#undef readint
#undef writeint

