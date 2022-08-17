/* Copyright (C)2012 Xiph.Org Foundation
   File: opus_header.h

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

#ifndef OPUS_HEADER_H
#define OPUS_HEADER_H

#include <stdlib.h>
#include <opus.h>

#include <opus_multistream.h>
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
#include <opus_projection.h>
#endif

typedef struct OpusGenericEncoder OpusGenericEncoder;
struct OpusGenericEncoder {
  OpusMSEncoder *ms;
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
  OpusProjectionEncoder *pr;
#endif
};

int opeint_use_projection(int channel_mapping);

int opeint_encoder_surround_init(OpusGenericEncoder *st, int Fs, int channels, int channel_mapping, int *nb_streams, int *nb_coupled, unsigned char *stream_map, int application);

void opeint_encoder_cleanup(OpusGenericEncoder *st);

int opeint_encoder_init(OpusGenericEncoder *st, opus_int32 Fs, int channels, int streams, int coupled_streams, const unsigned char *mapping, int application);

int opeint_encode_float(OpusGenericEncoder *st, const float *pcm, int frame_size, unsigned char *data, opus_int32 max_data_bytes);

#ifdef OPUS_HAVE_OPUS_PROJECTION_H
# define opeint_encoder_ctl(st, request) \
    ((st)->pr!=NULL ? \
    opus_projection_encoder_ctl((st)->pr, request) : \
    opus_multistream_encoder_ctl((st)->ms, request))
# define opeint_encoder_ctl2(st, request, value) \
    ((st)->pr!=NULL ? \
    opus_projection_encoder_ctl((st)->pr, request, value) : \
    opus_multistream_encoder_ctl((st)->ms, request, value))
#else
# define opeint_encoder_ctl(st, request) \
    opus_multistream_encoder_ctl((st)->ms, request)
# define opeint_encoder_ctl2(st, request, value) \
    opus_multistream_encoder_ctl((st)->ms, request, value)
#endif

typedef struct {
   int version;
   int channels; /* Number of channels: 1..255 */
   int preskip;
   opus_uint32 input_sample_rate;
   opus_int32 gain; /* in dB S7.8 should be zero whenever possible */
   int channel_mapping;
   /* The rest is only used if channel_mapping != 0 */
   int nb_streams;
   int nb_coupled;
   unsigned char stream_map[255];
} OpusHeader;

int opeint_opus_header_get_size(const OpusHeader *h);

int opeint_opus_header_to_packet(const OpusHeader *h, unsigned char *packet, int len, const OpusGenericEncoder *st);

void opeint_comment_init(char **comments, int* length, const char *vendor_string);

int opeint_comment_add(char **comments, int* length, const char *tag, const char *val);

void opeint_comment_pad(char **comments, int* length, int amount);

#endif
