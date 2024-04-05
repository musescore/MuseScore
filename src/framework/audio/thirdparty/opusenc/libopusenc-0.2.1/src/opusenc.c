/* Copyright (C)2002-2017 Jean-Marc Valin
   Copyright (C)2007-2013 Xiph.Org Foundation
   Copyright (C)2008-2013 Gregory Maxwell
   File: opusenc.c

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
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "opusenc.h"
#include "opus_header.h"
#include "speex_resampler.h"
#include "picture.h"
#include "ogg_packer.h"
#include "unicode_support.h"

/* Bump this when we change the ABI. */
#define OPE_ABI_VERSION 0

#define LPC_PADDING 120
#define LPC_ORDER 24
#define LPC_INPUT 480
/* Make the following constant always equal to 2*cos(M_PI/LPC_PADDING) */
#define LPC_GOERTZEL_CONST 1.99931465f

/* Allow up to 2 seconds for delayed decision. */
#define MAX_LOOKAHEAD 96000
/* We can't have a circular buffer (because of delayed decision), so let's not copy too often. */
#define BUFFER_EXTRA 24000

#define BUFFER_SAMPLES (MAX_LOOKAHEAD + BUFFER_EXTRA)

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#ifdef _MSC_VER
# if (_MSC_VER < 1900)
#  define snprintf _snprintf
# endif
#endif

struct StdioObject {
  FILE *file;
};

struct OggOpusComments {
  char *comment;
  int comment_length;
  int seen_file_icons;
};

/* Create a new comments object. The vendor string is optional. */
OggOpusComments *ope_comments_create() {
  OggOpusComments *c;
  char vendor_str[1024];
  c = malloc(sizeof(*c));
  if (c == NULL) return NULL;
  opeint_comment_init(&c->comment, &c->comment_length, vendor_str);
  c->seen_file_icons = 0;
  if (c->comment == NULL) {
    free(c);
    return NULL;
  } else {
    return c;
  }
}

/* Create a deep copy of a comments object. */
OggOpusComments *ope_comments_copy(OggOpusComments *comments) {
  OggOpusComments *c;
  c = malloc(sizeof(*c));
  if (c == NULL) return NULL;
  memcpy(c, comments, sizeof(*c));
  c->comment = malloc(comments->comment_length);
  if (c->comment == NULL) {
    free(c);
    return NULL;
  } else {
    memcpy(c->comment, comments->comment, comments->comment_length);
    return c;
  }
}

/* Destroys a comments object. */
void ope_comments_destroy(OggOpusComments *comments){
  free(comments->comment);
  free(comments);
}

/* Add a comment. */
int ope_comments_add(OggOpusComments *comments, const char *tag, const char *val) {
  if (tag == NULL || val == NULL) return OPE_BAD_ARG;
  if (strchr(tag, '=')) return OPE_BAD_ARG;
  if (opeint_comment_add(&comments->comment, &comments->comment_length, tag, val)) return OPE_ALLOC_FAIL;
  return OPE_OK;
}

/* Add a comment. */
int ope_comments_add_string(OggOpusComments *comments, const char *tag_and_val) {
  if (!strchr(tag_and_val, '=')) return OPE_BAD_ARG;
  if (opeint_comment_add(&comments->comment, &comments->comment_length, NULL, tag_and_val)) return OPE_ALLOC_FAIL;
  return OPE_OK;
}

int ope_comments_add_picture(OggOpusComments *comments, const char *filename, int picture_type, const char *description) {
  char *picture_data;
  int err;
  picture_data = opeint_parse_picture_specification(filename, picture_type, description, &err, &comments->seen_file_icons);
  if (picture_data == NULL || err != OPE_OK){
    return err;
  }
  opeint_comment_add(&comments->comment, &comments->comment_length, "METADATA_BLOCK_PICTURE", picture_data);
  free(picture_data);
  return OPE_OK;
}

int ope_comments_add_picture_from_memory(OggOpusComments *comments, const char *ptr, size_t size, int picture_type, const char *description) {
  char *picture_data;
  int err;
  picture_data = opeint_parse_picture_specification_from_memory(ptr, size, picture_type, description, &err, &comments->seen_file_icons);
  if (picture_data == NULL || err != OPE_OK){
    return err;
  }
  opeint_comment_add(&comments->comment, &comments->comment_length, "METADATA_BLOCK_PICTURE", picture_data);
  free(picture_data);
  return OPE_OK;
}


typedef struct EncStream EncStream;

struct EncStream {
  void *user_data;
  int serialno_is_set;
  int serialno;
  int stream_is_init;
  int packetno;
  char *comment;
  int comment_length;
  int seen_file_icons;
  int close_at_end;
  int header_is_frozen;
  opus_int64 end_granule;
  opus_int64 granule_offset;
  EncStream *next;
};

int opeint_use_projection(int channel_mapping) {
  if (channel_mapping==3){
    return 1;
  }
  return 0;
}

int opeint_encoder_surround_init(
    OpusGenericEncoder *st, int Fs, int channels, int channel_mapping,
    int *nb_streams, int *nb_coupled, unsigned char *stream_map, int application) {
  int ret;
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
  if(opeint_use_projection(channel_mapping)){
    int ci;
    st->pr=opus_projection_ambisonics_encoder_create(Fs, channels,
        channel_mapping, nb_streams, nb_coupled, application, &ret);
    for (ci = 0; ci < channels; ci++) {
      stream_map[ci] = ci;
    }
    st->ms=NULL;
  }
  else
#endif
  {
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
    st->pr=NULL;
#endif
    st->ms=opus_multistream_surround_encoder_create(Fs, channels,
        channel_mapping, nb_streams, nb_coupled, stream_map, application, &ret);
  }
  return ret;
}

void opeint_encoder_cleanup(OpusGenericEncoder *st) {
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
    if (st->pr) opus_projection_encoder_destroy(st->pr);
#endif
    if (st->ms) opus_multistream_encoder_destroy(st->ms);
}

int opeint_encoder_init(
    OpusGenericEncoder *st, opus_int32 Fs, int channels, int streams,
    int coupled_streams, const unsigned char *mapping, int application) {
  int ret;
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
  st->pr=NULL;
#endif
  st->ms=opus_multistream_encoder_create(Fs, channels, streams,
      coupled_streams, mapping, application, &ret);
  return ret;
}

int opeint_encode_float(
    OpusGenericEncoder *st,
    const float *pcm,
    int frame_size,
    unsigned char *data,
    opus_int32 max_data_bytes) {
  int ret;
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
  if (st->pr) ret=opus_projection_encode_float(st->pr, pcm, frame_size, data, max_data_bytes);
  else
#endif
    ret=opus_multistream_encode_float(st->ms, pcm, frame_size, data, max_data_bytes);
  return ret;
}

struct OggOpusEnc {
  OpusGenericEncoder st;
  oggpacker *oggp;
  int unrecoverable;
  int pull_api;
  int rate;
  int channels;
  float *buffer;
  int buffer_start;
  int buffer_end;
  SpeexResamplerState *re;
  int frame_size;
  int decision_delay;
  int max_ogg_delay;
  int global_granule_offset;
  opus_int64 curr_granule;
  opus_int64 write_granule;
  opus_int64 last_page_granule;
  int draining;
  int frame_size_request;
  float *lpc_buffer;
  unsigned char *chaining_keyframe;
  int chaining_keyframe_length;
  OpusEncCallbacks callbacks;
  ope_packet_func packet_callback;
  void *packet_callback_data;
  OpusHeader header;
  int comment_padding;
  EncStream *streams;
  EncStream *last_stream;
};

static int output_pages(OggOpusEnc *enc) {
  unsigned char *page;
  int len;
  while (oggp_get_next_page(enc->oggp, &page, &len)) {
    int ret = enc->callbacks.write(enc->streams->user_data, page, len);
    if (ret) return ret;
  }
  return 0;
}
static int oe_flush_page(OggOpusEnc *enc) {
  oggp_flush_page(enc->oggp);
  if (!enc->pull_api) return output_pages(enc);
  return 0;
}

static int stdio_write(void *user_data, const unsigned char *ptr, opus_int32 len) {
  int ret;
  struct StdioObject *obj = (struct StdioObject*)user_data;
  ret = fwrite(ptr, 1, len, obj->file) != (size_t)len;
  return ret;
}

static int stdio_close(void *user_data) {
  struct StdioObject *obj = (struct StdioObject*)user_data;
  int ret = 0;
  if (obj->file) ret = fclose(obj->file);
  free(obj);
  return ret!=0;
}

static const OpusEncCallbacks stdio_callbacks = {
  stdio_write,
  stdio_close
};

/* Create a new OggOpus file. */
OggOpusEnc *ope_encoder_create_file(const char *path, OggOpusComments *comments, opus_int32 rate, int channels, int family, int *error) {
  OggOpusEnc *enc;
  struct StdioObject *obj;
  obj = malloc(sizeof(*obj));
  if (obj == NULL) {
    if (error) *error = OPE_ALLOC_FAIL;
    return NULL;
  }
  enc = ope_encoder_create_callbacks(&stdio_callbacks, obj, comments, rate, channels, family, error);
  if (enc == NULL || (error && *error)) {
    free(obj);
    return NULL;
  }
  obj->file = opeint_fopen(path, "wb");
  if (!obj->file) {
    if (error) *error = OPE_CANNOT_OPEN;
    ope_encoder_destroy(enc);
    return NULL;
  }
  return enc;
}

EncStream *stream_create(OggOpusComments *comments) {
  EncStream *stream;
  stream = malloc(sizeof(*stream));
  if (!stream) return NULL;
  stream->next = NULL;
  stream->close_at_end = 1;
  stream->serialno_is_set = 0;
  stream->stream_is_init = 0;
  stream->header_is_frozen = 0;
  stream->granule_offset = 0;
  stream->comment = malloc(comments->comment_length);
  if (stream->comment == NULL) goto fail;
  memcpy(stream->comment, comments->comment, comments->comment_length);
  stream->comment_length = comments->comment_length;
  stream->seen_file_icons = comments->seen_file_icons;
  return stream;
fail:
  if (stream->comment) free(stream->comment);
  free(stream);
  return NULL;
}

static void stream_destroy(EncStream *stream) {
  if (stream->comment) free(stream->comment);
  free(stream);
}

/* Create a new OggOpus file (callback-based). */
OggOpusEnc *ope_encoder_create_callbacks(const OpusEncCallbacks *callbacks, void *user_data,
    OggOpusComments *comments, opus_int32 rate, int channels, int family, int *error) {
  OggOpusEnc *enc=NULL;
  int ret;
  if (family != 0 && family != 1 &&
#ifdef OPUS_HAVE_OPUS_PROJECTION_H
      family != 2 && family != 3 &&
#endif
      family != 255 && family != -1) {
    if (error) {
      if (family < -1 || family > 255) *error = OPE_BAD_ARG;
      else *error = OPE_UNIMPLEMENTED;
    }
    return NULL;
  }
  if (channels <= 0 || channels > 255) {
    if (error) *error = OPE_BAD_ARG;
    return NULL;
  }
  if (rate <= 0) {
    if (error) *error = OPE_BAD_ARG;
    return NULL;
  }
  /* Setting the most common failure up-front. */
  if (error) *error = OPE_ALLOC_FAIL;
  if ( (enc = malloc(sizeof(*enc))) == NULL) goto fail;
  enc->buffer = NULL;
  enc->lpc_buffer = NULL;
  if ( (enc->streams = stream_create(comments)) == NULL) goto fail;
  enc->last_stream = enc->streams;
  enc->oggp = NULL;
  /* Not initializing anything is an unrecoverable error. */
  enc->unrecoverable = family == -1 ? OPE_TOO_LATE : 0;
  enc->pull_api = 0;
  enc->packet_callback = NULL;
  enc->rate = rate;
  enc->channels = channels;
  enc->frame_size = 960;
  enc->decision_delay = 96000;
  enc->max_ogg_delay = 48000;
  enc->chaining_keyframe = NULL;
  enc->chaining_keyframe_length = -1;
  enc->comment_padding = 512;
  enc->header.channels=channels;
  enc->header.channel_mapping=family;
  enc->header.input_sample_rate=rate;
  enc->header.gain=0;
  if (family != -1) {
    ret=opeint_encoder_surround_init(&enc->st, 48000, channels,
        enc->header.channel_mapping, &enc->header.nb_streams,
        &enc->header.nb_coupled, enc->header.stream_map,
        OPUS_APPLICATION_AUDIO);
    if (! (ret == OPUS_OK) ) {
      if (ret == OPUS_BAD_ARG) ret = OPE_BAD_ARG;
      else if (ret == OPUS_INTERNAL_ERROR) ret = OPE_INTERNAL_ERROR;
      else if (ret == OPUS_UNIMPLEMENTED) ret = OPE_UNIMPLEMENTED;
      else if (ret == OPUS_ALLOC_FAIL) ret = OPE_ALLOC_FAIL;
      else ret = OPE_INTERNAL_ERROR;
      if (error) *error = ret;
      goto fail;
    }
    opeint_encoder_ctl(&enc->st, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_20_MS));
  }
  if (rate != 48000) {
    enc->re = speex_resampler_init(channels, rate, 48000, 5, NULL);
    if (enc->re == NULL) goto fail;
    speex_resampler_skip_zeros(enc->re);
  } else {
    enc->re = NULL;
  }
  enc->global_granule_offset = -1;
  enc->curr_granule = 0;
  enc->write_granule = 0;
  enc->last_page_granule = 0;
  enc->draining = 0;
  if ( (enc->buffer = malloc(sizeof(*enc->buffer)*BUFFER_SAMPLES*channels)) == NULL) goto fail;
  if (rate != 48000) {
    /* Allocate an extra LPC_PADDING samples so we can do the padding in-place. */
    if ( (enc->lpc_buffer = malloc(sizeof(*enc->lpc_buffer)*(LPC_INPUT+LPC_PADDING)*channels)) == NULL) goto fail;
    memset(enc->lpc_buffer, 0, sizeof(*enc->lpc_buffer)*LPC_INPUT*channels);
  }
  enc->buffer_start = enc->buffer_end = 0;
  if (callbacks != NULL)
  {
    enc->callbacks = *callbacks;
  }
  enc->streams->user_data = user_data;
  if (error) *error = OPE_OK;
  return enc;
fail:
  if (enc) {
    opeint_encoder_cleanup(&enc->st);
    if (enc->buffer) free(enc->buffer);
    if (enc->streams) stream_destroy(enc->streams);
    if (enc->lpc_buffer) free(enc->lpc_buffer);
    free(enc);
  }
  return NULL;
}

/* Create a new OggOpus stream, pulling one page at a time. */
OggOpusEnc *ope_encoder_create_pull(OggOpusComments *comments, opus_int32 rate, int channels, int family, int *error) {
  OggOpusEnc *enc = ope_encoder_create_callbacks(NULL, NULL, comments, rate, channels, family, error);
  if (enc) enc->pull_api = 1;
  return enc;
}

int ope_encoder_deferred_init_with_mapping(OggOpusEnc *enc, int family, int streams,
    int coupled_streams, const unsigned char *mapping) {
  int ret;
  int i;
  if (family < 0 || family > 255) return OPE_BAD_ARG;
  else if (family != 1 &&
  #ifdef OPUS_HAVE_OPUS_PROJECTION_H
      family != 2 &&
  #endif
      family != 255) return OPE_UNIMPLEMENTED;
  else if (streams <= 0 || streams>255 || coupled_streams<0 || coupled_streams >= 128 || streams+coupled_streams > 255) return OPE_BAD_ARG;
  ret=opeint_encoder_init(&enc->st, 48000, enc->channels, streams, coupled_streams, mapping, OPUS_APPLICATION_AUDIO);
  if (! (ret == OPUS_OK) ) {
    if (ret == OPUS_BAD_ARG) ret = OPE_BAD_ARG;
    else if (ret == OPUS_INTERNAL_ERROR) ret = OPE_INTERNAL_ERROR;
    else if (ret == OPUS_UNIMPLEMENTED) ret = OPE_UNIMPLEMENTED;
    else if (ret == OPUS_ALLOC_FAIL) ret = OPE_ALLOC_FAIL;
    else ret = OPE_INTERNAL_ERROR;
    return ret;
  }
  opeint_encoder_ctl(&enc->st, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_20_MS));
  enc->unrecoverable = 0;
  enc->header.channel_mapping=family;
  enc->header.nb_streams = streams;
  enc->header.nb_coupled = coupled_streams;
  for (i=0;i<streams+coupled_streams;i++)
    enc->header.stream_map[i] = mapping[i];
  return OPE_OK;
}

static void init_stream(OggOpusEnc *enc) {
  assert(!enc->streams->stream_is_init);
  if (!enc->streams->serialno_is_set) {
    enc->streams->serialno = rand();
  }

  if (enc->oggp != NULL) oggp_chain(enc->oggp, enc->streams->serialno);
  else {
    enc->oggp = oggp_create(enc->streams->serialno);
    if (enc->oggp == NULL) {
      enc->unrecoverable = OPE_ALLOC_FAIL;
      return;
    }
    oggp_set_muxing_delay(enc->oggp, enc->max_ogg_delay);
  }
  opeint_comment_pad(&enc->streams->comment, &enc->streams->comment_length, enc->comment_padding);

  /* Get preskip at the last minute (when it can no longer change). */
  if (enc->global_granule_offset == -1) {
    opus_int32 tmp;
    int ret;
    ret=opeint_encoder_ctl(&enc->st, OPUS_GET_LOOKAHEAD(&tmp));
    if (ret == OPUS_OK) enc->header.preskip = tmp;
    else enc->header.preskip = 0;
    enc->global_granule_offset = enc->header.preskip;
  }
  /*Write header*/
  {
    int header_size;
    int ret;
    int packet_size;
    unsigned char *p;
    header_size = opeint_opus_header_get_size(&enc->header);
    p = oggp_get_packet_buffer(enc->oggp, header_size);
    packet_size = opeint_opus_header_to_packet(&enc->header, p, header_size, &enc->st);
    if (enc->packet_callback) enc->packet_callback(enc->packet_callback_data, p, packet_size, 0);
    oggp_commit_packet(enc->oggp, packet_size, 0, 0);
    ret = oe_flush_page(enc);
    if (ret) {
      enc->unrecoverable = OPE_WRITE_FAIL;
      return;
    }
    p = oggp_get_packet_buffer(enc->oggp, enc->streams->comment_length);
    memcpy(p, enc->streams->comment, enc->streams->comment_length);
    if (enc->packet_callback) enc->packet_callback(enc->packet_callback_data, p, enc->streams->comment_length, 0);
    oggp_commit_packet(enc->oggp, enc->streams->comment_length, 0, 0);
    ret = oe_flush_page(enc);
    if (ret) {
      enc->unrecoverable = OPE_WRITE_FAIL;
      return;
    }
  }
  enc->streams->stream_is_init = 1;
  enc->streams->packetno = 2;
}

static void shift_buffer(OggOpusEnc *enc) {
  /* Leaving enough in the buffer to do LPC extension if needed. */
  if (enc->buffer_start > LPC_INPUT) {
    memmove(&enc->buffer[0], &enc->buffer[enc->channels*(enc->buffer_start-LPC_INPUT)],
            enc->channels*(enc->buffer_end-enc->buffer_start+LPC_INPUT)*sizeof(*enc->buffer));
    enc->buffer_end -= enc->buffer_start-LPC_INPUT;
    enc->buffer_start = LPC_INPUT;
  }
}

static int compute_frame_samples(int size_request) {
  if (size_request <= OPUS_FRAMESIZE_40_MS) return 120<<(size_request-OPUS_FRAMESIZE_2_5_MS);
  else return (size_request-OPUS_FRAMESIZE_2_5_MS-2)*960;
}

static void encode_buffer(OggOpusEnc *enc) {
  opus_int32 max_packet_size;
  /* Round up when converting the granule pos because the decoder will round down. */
  opus_int64 end_granule48k = (enc->streams->end_granule*48000 + enc->rate - 1)/enc->rate + enc->global_granule_offset;
  max_packet_size = (1277*6+2)*enc->header.nb_streams;
  while (enc->buffer_end-enc->buffer_start > enc->frame_size + enc->decision_delay) {
    int cont;
    int e_o_s;
    opus_int32 pred;
    int nbBytes;
    unsigned char *packet;
    unsigned char *packet_copy = NULL;
    int is_keyframe=0;
    if (enc->unrecoverable) return;
    opeint_encoder_ctl(&enc->st, OPUS_GET_PREDICTION_DISABLED(&pred));
    /* FIXME: a frame that follows a keyframe generally doesn't need to be a keyframe
       unless there's two consecutive stream boundaries. */
    if (enc->curr_granule + 2*enc->frame_size>= end_granule48k && enc->streams->next) {
      opeint_encoder_ctl(&enc->st, OPUS_SET_PREDICTION_DISABLED(1));
      is_keyframe = 1;
    }
    /* Handle the last packet by making sure not to encode too much padding. */
    if (enc->curr_granule+enc->frame_size >= end_granule48k && enc->draining && enc->frame_size_request > OPUS_FRAMESIZE_20_MS) {
      int min_samples;
      int frame_size_request = OPUS_FRAMESIZE_20_MS;
      /* Minimum frame size required for the current frame to still meet the e_o_s condition. */
      min_samples = end_granule48k - enc->curr_granule;
      while (compute_frame_samples(frame_size_request) < min_samples) frame_size_request++;
      assert(frame_size_request <= enc->frame_size_request);
      ope_encoder_ctl(enc, OPUS_SET_EXPERT_FRAME_DURATION(frame_size_request));
    }
    packet = oggp_get_packet_buffer(enc->oggp, max_packet_size);
    nbBytes = opeint_encode_float(&enc->st, &enc->buffer[enc->channels*enc->buffer_start],
        enc->buffer_end-enc->buffer_start, packet, max_packet_size);
    if (nbBytes < 0) {
      /* Anything better we can do here? */
      enc->unrecoverable = OPE_INTERNAL_ERROR;
      return;
    }
    opeint_encoder_ctl(&enc->st, OPUS_SET_PREDICTION_DISABLED(pred));
    assert(nbBytes > 0);
    enc->curr_granule += enc->frame_size;
    do {
      int ret;
      opus_int64 granulepos;
      granulepos=enc->curr_granule-enc->streams->granule_offset;
      e_o_s=enc->curr_granule >= end_granule48k;
      cont = 0;
      if (e_o_s) granulepos=end_granule48k-enc->streams->granule_offset;
      if (packet_copy != NULL) {
        packet = oggp_get_packet_buffer(enc->oggp, max_packet_size);
        memcpy(packet, packet_copy, nbBytes);
      }
      if (enc->packet_callback) enc->packet_callback(enc->packet_callback_data, packet, nbBytes, 0);
      if ((e_o_s || is_keyframe) && packet_copy == NULL) {
        packet_copy = malloc(nbBytes);
        if (packet_copy == NULL) {
          /* Can't recover from allocation failing here. */
          enc->unrecoverable = OPE_ALLOC_FAIL;
          return;
        }
        memcpy(packet_copy, packet, nbBytes);
      }
      oggp_commit_packet(enc->oggp, nbBytes, granulepos, e_o_s);
      if (e_o_s) ret = oe_flush_page(enc);
      else if (!enc->pull_api) ret = output_pages(enc);
      else ret = 0;
      if (ret) {
        enc->unrecoverable = OPE_WRITE_FAIL;
        if (packet_copy) free(packet_copy);
        return;
      }
      if (e_o_s) {
        EncStream *tmp;
        tmp = enc->streams->next;
        if (enc->streams->close_at_end && !enc->pull_api) {
          ret = enc->callbacks.close(enc->streams->user_data);
          if (ret) {
            enc->unrecoverable = OPE_CLOSE_FAIL;
            free(packet_copy);
            return;
          }
        }
        stream_destroy(enc->streams);
        enc->streams = tmp;
        if (!tmp) enc->last_stream = NULL;
        if (enc->last_stream == NULL) {
          free(packet_copy);
          return;
        }
        /* We're done with this stream, start the next one. */
        enc->header.preskip = end_granule48k + enc->frame_size - enc->curr_granule;
        enc->streams->granule_offset = enc->curr_granule - enc->frame_size;
        if (enc->chaining_keyframe) {
          enc->header.preskip += enc->frame_size;
          enc->streams->granule_offset -= enc->frame_size;
        }
        init_stream(enc);
        if (enc->chaining_keyframe) {
          unsigned char *p;
          opus_int64 granulepos2=enc->curr_granule - enc->streams->granule_offset - enc->frame_size;
          p = oggp_get_packet_buffer(enc->oggp, enc->chaining_keyframe_length);
          memcpy(p, enc->chaining_keyframe, enc->chaining_keyframe_length);
          if (enc->packet_callback) enc->packet_callback(enc->packet_callback_data, enc->chaining_keyframe, enc->chaining_keyframe_length, 0);
          oggp_commit_packet(enc->oggp, enc->chaining_keyframe_length, granulepos2, 0);
        }
        end_granule48k = (enc->streams->end_granule*48000 + enc->rate - 1)/enc->rate + enc->global_granule_offset;
        cont = 1;
      }
    } while (cont);
    if (enc->chaining_keyframe) free(enc->chaining_keyframe);
    if (is_keyframe) {
      enc->chaining_keyframe_length = nbBytes;
      enc->chaining_keyframe = packet_copy;
      packet_copy = NULL;
    } else {
      enc->chaining_keyframe = NULL;
      enc->chaining_keyframe_length = -1;
    }
    if (packet_copy) free(packet_copy);
    enc->buffer_start += enc->frame_size;
  }
  /* If we've reached the end of the buffer, move everything back to the front. */
  if (enc->buffer_end == BUFFER_SAMPLES) {
    shift_buffer(enc);
  }
  /* This function must never leave the buffer full. */
  assert(enc->buffer_end < BUFFER_SAMPLES);
}

/* Add/encode any number of float samples to the file. */
int ope_encoder_write_float(OggOpusEnc *enc, const float *pcm, int samples_per_channel) {
  int channels = enc->channels;
  if (enc->unrecoverable) return enc->unrecoverable;
  enc->last_stream->header_is_frozen = 1;
  if (!enc->streams->stream_is_init) init_stream(enc);
  if (samples_per_channel < 0) return OPE_BAD_ARG;
  enc->write_granule += samples_per_channel;
  enc->last_stream->end_granule = enc->write_granule;
  if (enc->lpc_buffer) {
    int i;
    if (samples_per_channel < LPC_INPUT) {
      for (i=0;i<(LPC_INPUT-samples_per_channel)*channels;i++) enc->lpc_buffer[i] = enc->lpc_buffer[samples_per_channel*channels + i];
      for (i=0;i<samples_per_channel*channels;i++) enc->lpc_buffer[(LPC_INPUT-samples_per_channel)*channels + i] = pcm[i];
    } else {
      for (i=0;i<LPC_INPUT*channels;i++) enc->lpc_buffer[i] = pcm[(samples_per_channel-LPC_INPUT)*channels + i];
    }
  }
  do {
    int i;
    spx_uint32_t in_samples, out_samples;
    out_samples = BUFFER_SAMPLES-enc->buffer_end;
    if (enc->re != NULL) {
      in_samples = samples_per_channel;
      speex_resampler_process_interleaved_float(enc->re, pcm, &in_samples, &enc->buffer[channels*enc->buffer_end], &out_samples);
    } else {
      int curr;
      curr = MIN((spx_uint32_t)samples_per_channel, out_samples);
      for (i=0;i<channels*curr;i++) {
      enc->buffer[channels*enc->buffer_end+i] = pcm[i];
      }
      in_samples = out_samples = curr;
    }
    enc->buffer_end += out_samples;
    pcm += in_samples*channels;
    samples_per_channel -= in_samples;
    encode_buffer(enc);
    if (enc->unrecoverable) return enc->unrecoverable;
  } while (samples_per_channel > 0);
  return OPE_OK;
}

#define CONVERT_BUFFER 4096

/* Add/encode any number of int16 samples to the file. */
int ope_encoder_write(OggOpusEnc *enc, const opus_int16 *pcm, int samples_per_channel) {
  int channels = enc->channels;
  if (enc->unrecoverable) return enc->unrecoverable;
  enc->last_stream->header_is_frozen = 1;
  if (!enc->streams->stream_is_init) init_stream(enc);
  if (samples_per_channel < 0) return OPE_BAD_ARG;
  enc->write_granule += samples_per_channel;
  enc->last_stream->end_granule = enc->write_granule;
  if (enc->lpc_buffer) {
    int i;
    if (samples_per_channel < LPC_INPUT) {
      for (i=0;i<(LPC_INPUT-samples_per_channel)*channels;i++) enc->lpc_buffer[i] = enc->lpc_buffer[samples_per_channel*channels + i];
      for (i=0;i<samples_per_channel*channels;i++) enc->lpc_buffer[(LPC_INPUT-samples_per_channel)*channels + i] = (1.f/32768)*pcm[i];
    } else {
      for (i=0;i<LPC_INPUT*channels;i++) enc->lpc_buffer[i] = (1.f/32768)*pcm[(samples_per_channel-LPC_INPUT)*channels + i];
    }
  }
  do {
    int i;
    spx_uint32_t in_samples, out_samples;
    out_samples = BUFFER_SAMPLES-enc->buffer_end;
    if (enc->re != NULL) {
      float buf[CONVERT_BUFFER];
      in_samples = MIN(CONVERT_BUFFER/channels, samples_per_channel);
      for (i=0;i<channels*(int)in_samples;i++) {
        buf[i] = (1.f/32768)*pcm[i];
      }
      speex_resampler_process_interleaved_float(enc->re, buf, &in_samples, &enc->buffer[channels*enc->buffer_end], &out_samples);
    } else {
      int curr;
      curr = MIN((spx_uint32_t)samples_per_channel, out_samples);
      for (i=0;i<channels*curr;i++) {
        enc->buffer[channels*enc->buffer_end+i] = (1.f/32768)*pcm[i];
      }
      in_samples = out_samples = curr;
    }
    enc->buffer_end += out_samples;
    pcm += in_samples*channels;
    samples_per_channel -= in_samples;
    encode_buffer(enc);
    if (enc->unrecoverable) return enc->unrecoverable;
  } while (samples_per_channel > 0);
  return OPE_OK;
}

/* Get the next page from the stream. Returns 1 if there is a page available, 0 if not. */
int ope_encoder_get_page(OggOpusEnc *enc, unsigned char **page, opus_int32 *len, int flush) {
  if (enc->unrecoverable) return enc->unrecoverable;
  if (!enc->pull_api) return 0;
  else {
    if (flush) oggp_flush_page(enc->oggp);
    return oggp_get_next_page(enc->oggp, page, len);
  }
}

static void extend_signal(float *x, int before, int after, int channels);

int ope_encoder_drain(OggOpusEnc *enc) {
  int pad_samples;
  int resampler_drain = 0;
  if (enc->unrecoverable) return enc->unrecoverable;
  /* Check if it's already been drained. */
  if (enc->streams == NULL) return OPE_TOO_LATE;
  if (enc->re) resampler_drain = speex_resampler_get_output_latency(enc->re);
  pad_samples = MAX(LPC_PADDING, enc->global_granule_offset + enc->frame_size + resampler_drain + 1);
  if (!enc->streams->stream_is_init) init_stream(enc);
  shift_buffer(enc);
  assert(enc->buffer_end + pad_samples <= BUFFER_SAMPLES);
  memset(&enc->buffer[enc->channels*enc->buffer_end], 0, pad_samples*enc->channels*sizeof(enc->buffer[0]));
  if (enc->re) {
    spx_uint32_t in_samples, out_samples;
    extend_signal(&enc->lpc_buffer[LPC_INPUT*enc->channels], LPC_INPUT, LPC_PADDING, enc->channels);
    do {
      in_samples = LPC_PADDING;
      out_samples = pad_samples;
      speex_resampler_process_interleaved_float(enc->re, &enc->lpc_buffer[LPC_INPUT*enc->channels], &in_samples, &enc->buffer[enc->channels*enc->buffer_end], &out_samples);
      enc->buffer_end += out_samples;
      pad_samples -= out_samples;
      /* If we don't have enough padding, zero all zeros and repeat. */
      memset(&enc->lpc_buffer[LPC_INPUT*enc->channels], 0, LPC_PADDING*enc->channels*sizeof(enc->lpc_buffer[0]));
    } while (pad_samples > 0);
  } else {
    extend_signal(&enc->buffer[enc->channels*enc->buffer_end], enc->buffer_end, LPC_PADDING, enc->channels);
    enc->buffer_end += pad_samples;
  }
  enc->decision_delay = 0;
  enc->draining = 1;
  assert(enc->buffer_end <= BUFFER_SAMPLES);
  encode_buffer(enc);
  if (enc->unrecoverable) return enc->unrecoverable;
  /* Draining should have called all the streams to complete. */
  assert(enc->streams == NULL);
  return OPE_OK;
}

void ope_encoder_destroy(OggOpusEnc *enc) {
  EncStream *stream;
  stream = enc->streams;
  while (stream != NULL) {
    EncStream *tmp = stream;
    stream = stream->next;
    /* Ignore any error on close. */
    if (tmp->close_at_end && !enc->pull_api) enc->callbacks.close(tmp->user_data);
    stream_destroy(tmp);
  }
  if (enc->chaining_keyframe) free(enc->chaining_keyframe);
  free(enc->buffer);
  if (enc->oggp) oggp_destroy(enc->oggp);
  opeint_encoder_cleanup(&enc->st);
  if (enc->re) speex_resampler_destroy(enc->re);
  if (enc->lpc_buffer) free(enc->lpc_buffer);
  free(enc);
}

/* Ends the stream and create a new stream within the same file. */
int ope_encoder_chain_current(OggOpusEnc *enc, OggOpusComments *comments) {
  enc->last_stream->close_at_end = 0;
  return ope_encoder_continue_new_callbacks(enc, enc->last_stream->user_data, comments);
}

/* Ends the stream and create a new file. */
int ope_encoder_continue_new_file(OggOpusEnc *enc, const char *path, OggOpusComments *comments) {
  int ret;
  struct StdioObject *obj;
  if (!(obj = malloc(sizeof(*obj)))) return OPE_ALLOC_FAIL;
  obj->file = opeint_fopen(path, "wb");
  if (!obj->file) {
    free(obj);
    /* By trying to open the file first, we can recover if we can't open it. */
    return OPE_CANNOT_OPEN;
  }
  ret = ope_encoder_continue_new_callbacks(enc, obj, comments);
  if (ret == OPE_OK) return ret;
  fclose(obj->file);
  free(obj);
  return ret;
}

/* Ends the stream and create a new file (callback-based). */
int ope_encoder_continue_new_callbacks(OggOpusEnc *enc, void *user_data, OggOpusComments *comments) {
  EncStream *new_stream;
  if (enc->unrecoverable) return enc->unrecoverable;
  assert(enc->streams);
  assert(enc->last_stream);
  new_stream = stream_create(comments);
  if (!new_stream) return OPE_ALLOC_FAIL;
  new_stream->user_data = user_data;
  new_stream->end_granule = enc->write_granule;
  enc->last_stream->next = new_stream;
  enc->last_stream = new_stream;
  return OPE_OK;
}

int ope_encoder_flush_header(OggOpusEnc *enc) {
  if (enc->unrecoverable) return enc->unrecoverable;
  if (enc->last_stream->header_is_frozen) return OPE_TOO_LATE;
  if (enc->last_stream->stream_is_init) return OPE_TOO_LATE;
  else init_stream(enc);
  return OPE_OK;
}

/* Goes straight to the libopus ctl() functions. */
int ope_encoder_ctl(OggOpusEnc *enc, int request, ...) {
  int ret;
  int translate;
  va_list ap;
  if (enc->unrecoverable) return enc->unrecoverable;
  va_start(ap, request);
  ret = OPE_OK;
  switch (request) {
    case OPUS_SET_APPLICATION_REQUEST:
    case OPUS_SET_BITRATE_REQUEST:
    case OPUS_SET_MAX_BANDWIDTH_REQUEST:
    case OPUS_SET_VBR_REQUEST:
    case OPUS_SET_BANDWIDTH_REQUEST:
    case OPUS_SET_COMPLEXITY_REQUEST:
    case OPUS_SET_INBAND_FEC_REQUEST:
    case OPUS_SET_PACKET_LOSS_PERC_REQUEST:
    case OPUS_SET_DTX_REQUEST:
    case OPUS_SET_VBR_CONSTRAINT_REQUEST:
    case OPUS_SET_FORCE_CHANNELS_REQUEST:
    case OPUS_SET_SIGNAL_REQUEST:
    case OPUS_SET_LSB_DEPTH_REQUEST:
    case OPUS_SET_PREDICTION_DISABLED_REQUEST:
#ifdef OPUS_SET_PHASE_INVERSION_DISABLED_REQUEST
    case OPUS_SET_PHASE_INVERSION_DISABLED_REQUEST:
#endif
    {
      opus_int32 value = va_arg(ap, opus_int32);
      ret = opeint_encoder_ctl2(&enc->st, request, value);
    }
    break;
    case OPUS_GET_LOOKAHEAD_REQUEST:
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      ret = opeint_encoder_ctl(&enc->st, OPUS_GET_LOOKAHEAD(value));
    }
    break;
    case OPUS_SET_EXPERT_FRAME_DURATION_REQUEST:
    {
      opus_int32 value = va_arg(ap, opus_int32);
      int max_supported = OPUS_FRAMESIZE_60_MS;
#ifdef OPUS_FRAMESIZE_120_MS
      max_supported = OPUS_FRAMESIZE_120_MS;
#endif
      if (value < OPUS_FRAMESIZE_2_5_MS || value > max_supported) {
        ret = OPUS_UNIMPLEMENTED;
        break;
      }
      ret = opeint_encoder_ctl(&enc->st, OPUS_SET_EXPERT_FRAME_DURATION(value));
      if (ret == OPUS_OK) {
        enc->frame_size = compute_frame_samples(value);
        enc->frame_size_request = value;
      }
    }
    break;
    case OPUS_GET_APPLICATION_REQUEST:
    case OPUS_GET_BITRATE_REQUEST:
    case OPUS_GET_MAX_BANDWIDTH_REQUEST:
    case OPUS_GET_VBR_REQUEST:
    case OPUS_GET_BANDWIDTH_REQUEST:
    case OPUS_GET_COMPLEXITY_REQUEST:
    case OPUS_GET_INBAND_FEC_REQUEST:
    case OPUS_GET_PACKET_LOSS_PERC_REQUEST:
    case OPUS_GET_DTX_REQUEST:
    case OPUS_GET_VBR_CONSTRAINT_REQUEST:
    case OPUS_GET_FORCE_CHANNELS_REQUEST:
    case OPUS_GET_SIGNAL_REQUEST:
    case OPUS_GET_LSB_DEPTH_REQUEST:
    case OPUS_GET_PREDICTION_DISABLED_REQUEST:
#ifdef OPUS_GET_PHASE_INVERSION_DISABLED_REQUEST
    case OPUS_GET_PHASE_INVERSION_DISABLED_REQUEST:
#endif
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      ret = opeint_encoder_ctl2(&enc->st, request, value);
    }
    break;
    case OPUS_MULTISTREAM_GET_ENCODER_STATE_REQUEST:
    {
      opus_int32 stream_id;
      OpusEncoder **value;
      stream_id = va_arg(ap, opus_int32);
      value = va_arg(ap, OpusEncoder**);
      opeint_encoder_ctl(&enc->st, OPUS_MULTISTREAM_GET_ENCODER_STATE(stream_id, value));
    }
    break;

    /* ****************** libopusenc-specific requests. ********************** */
    case OPE_SET_DECISION_DELAY_REQUEST:
    {
      opus_int32 value = va_arg(ap, opus_int32);
      if (value < 0) {
        ret = OPE_BAD_ARG;
        break;
      }
      value = MIN(value, MAX_LOOKAHEAD);
      enc->decision_delay = value;
    }
    break;
    case OPE_GET_DECISION_DELAY_REQUEST:
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      *value = enc->decision_delay;
    }
    break;
    case OPE_SET_MUXING_DELAY_REQUEST:
    {
      opus_int32 value = va_arg(ap, opus_int32);
      if (value < 0) {
        ret = OPE_BAD_ARG;
        break;
      }
      enc->max_ogg_delay = value;
      if (enc->oggp) oggp_set_muxing_delay(enc->oggp, enc->max_ogg_delay);
    }
    break;
    case OPE_GET_MUXING_DELAY_REQUEST:
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      *value = enc->max_ogg_delay;
    }
    break;
    case OPE_SET_COMMENT_PADDING_REQUEST:
    {
      opus_int32 value = va_arg(ap, opus_int32);
      if (value < 0) {
        ret = OPE_BAD_ARG;
        break;
      }
      enc->comment_padding = value;
      ret = OPE_OK;
    }
    break;
    case OPE_GET_COMMENT_PADDING_REQUEST:
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      *value = enc->comment_padding;
    }
    break;
    case OPE_SET_SERIALNO_REQUEST:
    {
      opus_int32 value = va_arg(ap, opus_int32);
      if (!enc->last_stream || enc->last_stream->header_is_frozen) {
        ret = OPE_TOO_LATE;
        break;
      }
      enc->last_stream->serialno = value;
      enc->last_stream->serialno_is_set = 1;
      ret = OPE_OK;
    }
    break;
    case OPE_GET_SERIALNO_REQUEST:
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      *value = enc->last_stream->serialno;
    }
    break;
    case OPE_SET_PACKET_CALLBACK_REQUEST:
    {
      ope_packet_func value = va_arg(ap, ope_packet_func);
      void *data = va_arg(ap, void *);
      enc->packet_callback = value;
      enc->packet_callback_data = data;
      ret = OPE_OK;
    }
    break;
    case OPE_SET_HEADER_GAIN_REQUEST:
    {
      opus_int32 value = va_arg(ap, opus_int32);
      if (!enc->last_stream || enc->last_stream->header_is_frozen) {
        ret = OPE_TOO_LATE;
        break;
      }
      enc->header.gain = value;
      ret = OPE_OK;
    }
    break;
    case OPE_GET_HEADER_GAIN_REQUEST:
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      *value = enc->header.gain;
    }
    break;
    case OPE_GET_NB_STREAMS_REQUEST:
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      *value = enc->header.nb_streams;
    }
    break;
    case OPE_GET_NB_COUPLED_STREAMS_REQUEST:
    {
      opus_int32 *value = va_arg(ap, opus_int32*);
      *value = enc->header.nb_coupled;
    }
    break;
    default:
      ret = OPUS_UNIMPLEMENTED;
  }
  va_end(ap);
  translate = ret != 0 && (request < 14000 || (ret < 0 && ret >= -10));
  if (translate) {
    if (ret == OPUS_BAD_ARG) ret = OPE_BAD_ARG;
    else if (ret == OPUS_INTERNAL_ERROR) ret = OPE_INTERNAL_ERROR;
    else if (ret == OPUS_UNIMPLEMENTED) ret = OPE_UNIMPLEMENTED;
    else if (ret == OPUS_ALLOC_FAIL) ret = OPE_ALLOC_FAIL;
    else ret = OPE_INTERNAL_ERROR;
  }
  assert(ret == 0 || ret < -10);
  return ret;
}

const char *ope_strerror(int error) {
  static const char * const ope_error_strings[] = {
    "cannot open file",
    "call cannot be made at this point",
    "invalid picture file",
    "invalid icon file (pictures of type 1 MUST be 32x32 PNGs)",
    "write failed",
    "close failed"
  };
  if (error == 0) return "success";
  else if (error >= -10) return "unknown error";
  else if (error > -30) return opus_strerror(error+10);
  else if (error >= OPE_CLOSE_FAIL) return ope_error_strings[-error-30];
  else return "unknown error";
}

const char *ope_get_version_string(void)
{
  return "libopusenc ";
}

int ope_get_abi_version(void) {
  return OPE_ABI_VERSION;
}

static void vorbis_lpc_from_data(float *data, float *lpci, int n, int stride);

static void extend_signal(float *x, int before, int after, int channels) {
  int c;
  int i;
  float window[LPC_PADDING];
  if (after==0) return;
  before = MIN(before, LPC_INPUT);
  if (before < 4*LPC_ORDER) {
    int i;
    for (i=0;i<after*channels;i++) x[i] = 0;
    return;
  }
  {
    /* Generate Window using a resonating IIR aka Goertzel's algorithm. */
    float m0=1, m1=.5*LPC_GOERTZEL_CONST;
    float a1 = LPC_GOERTZEL_CONST;
    window[0] = 1;
    for (i=1;i<LPC_PADDING;i++) {
      window[i] = a1*m0 - m1;
      m1 = m0;
      m0 = window[i];
    }
    for (i=0;i<LPC_PADDING;i++) window[i] = .5+.5*window[i];
  }
  for (c=0;c<channels;c++) {
    float lpc[LPC_ORDER];
    vorbis_lpc_from_data(x-channels*before+c, lpc, before, channels);
    for (i=0;i<after;i++) {
      float sum;
      int j;
      sum = 0;
      for (j=0;j<LPC_ORDER;j++) sum -= x[(i-j-1)*channels + c]*lpc[j];
      x[i*channels + c] = sum;
    }
    for (i=0;i<after;i++) x[i*channels + c] *= window[i];
  }
}

/* Some of these routines (autocorrelator, LPC coefficient estimator)
   are derived from code written by Jutta Degener and Carsten Bormann;
   thus we include their copyright below.  The entirety of this file
   is freely redistributable on the condition that both of these
   copyright notices are preserved without modification.  */

/* Preserved Copyright: *********************************************/

/* Copyright 1992, 1993, 1994 by Jutta Degener and Carsten Bormann,
Technische Universita"t Berlin

Any use of this software is permitted provided that this notice is not
removed and that neither the authors nor the Technische Universita"t
Berlin are deemed to have made any representations as to the
suitability of this software for any purpose nor are held responsible
for any defects of this software. THERE IS ABSOLUTELY NO WARRANTY FOR
THIS SOFTWARE.

As a matter of courtesy, the authors request to be informed about uses
this software has found, about bugs in this software, and about any
improvements that may be of general interest.

Berlin, 28.11.1994
Jutta Degener
Carsten Bormann

*********************************************************************/

static void vorbis_lpc_from_data(float *data, float *lpci, int n, int stride) {
  double aut[LPC_ORDER+1];
  double lpc[LPC_ORDER];
  double error;
  double epsilon;
  int i,j;

  /* FIXME: Apply a window to the input. */
  /* autocorrelation, p+1 lag coefficients */
  j=LPC_ORDER+1;
  while(j--){
    double d=0; /* double needed for accumulator depth */
    for(i=j;i<n;i++)d+=(double)data[i*stride]*data[(i-j)*stride];
    aut[j]=d;
  }

  /* Apply lag windowing (better than bandwidth expansion) */
  if (LPC_ORDER <= 64) {
    for (i=1;i<=LPC_ORDER;i++) {
      /* Approximate this gaussian for low enough order. */
      /* aut[i] *= exp(-.5*(2*M_PI*.002*i)*(2*M_PI*.002*i));*/
      aut[i] -= aut[i]*(0.008f*0.008f)*i*i;
    }
  }
  /* Generate lpc coefficients from autocorr values */

  /* set our noise floor to about -100dB */
  error=aut[0] * (1. + 1e-7);
  epsilon=1e-6*aut[0]+1e-7;

  for(i=0;i<LPC_ORDER;i++){
    double r= -aut[i+1];

    if(error<epsilon){
      memset(lpc+i,0,(LPC_ORDER-i)*sizeof(*lpc));
      goto done;
    }

    /* Sum up this iteration's reflection coefficient; note that in
       Vorbis we don't save it.  If anyone wants to recycle this code
       and needs reflection coefficients, save the results of 'r' from
       each iteration. */

    for(j=0;j<i;j++)r-=lpc[j]*aut[i-j];
    r/=error;

    /* Update LPC coefficients and total error */

    lpc[i]=r;
    for(j=0;j<i/2;j++){
      double tmp=lpc[j];

      lpc[j]+=r*lpc[i-1-j];
      lpc[i-1-j]+=r*tmp;
    }
    if(i&1)lpc[j]+=lpc[j]*r;

    error*=1.-r*r;

  }

 done:

  /* slightly damp the filter */
  {
    double g = .999;
    double damp = g;
    for(j=0;j<LPC_ORDER;j++){
      lpc[j]*=damp;
      damp*=g;
    }
  }

  for(j=0;j<LPC_ORDER;j++)lpci[j]=(float)lpc[j];
}

