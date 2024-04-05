/* Copyright (c) 2017 Jean-Marc Valin
   Copyright (c) 1994-2010 Xiph.Org Foundation */
/*
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

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>
#include "ogg_packer.h"

#define MAX_HEADER_SIZE (27+255)

#define MAX_PAGE_SIZE (255*255 + MAX_HEADER_SIZE)

static const oggp_uint32 crc_lookup[256]={
  0x00000000,0x04c11db7,0x09823b6e,0x0d4326d9,
  0x130476dc,0x17c56b6b,0x1a864db2,0x1e475005,
  0x2608edb8,0x22c9f00f,0x2f8ad6d6,0x2b4bcb61,
  0x350c9b64,0x31cd86d3,0x3c8ea00a,0x384fbdbd,
  0x4c11db70,0x48d0c6c7,0x4593e01e,0x4152fda9,
  0x5f15adac,0x5bd4b01b,0x569796c2,0x52568b75,
  0x6a1936c8,0x6ed82b7f,0x639b0da6,0x675a1011,
  0x791d4014,0x7ddc5da3,0x709f7b7a,0x745e66cd,
  0x9823b6e0,0x9ce2ab57,0x91a18d8e,0x95609039,
  0x8b27c03c,0x8fe6dd8b,0x82a5fb52,0x8664e6e5,
  0xbe2b5b58,0xbaea46ef,0xb7a96036,0xb3687d81,
  0xad2f2d84,0xa9ee3033,0xa4ad16ea,0xa06c0b5d,
  0xd4326d90,0xd0f37027,0xddb056fe,0xd9714b49,
  0xc7361b4c,0xc3f706fb,0xceb42022,0xca753d95,
  0xf23a8028,0xf6fb9d9f,0xfbb8bb46,0xff79a6f1,
  0xe13ef6f4,0xe5ffeb43,0xe8bccd9a,0xec7dd02d,
  0x34867077,0x30476dc0,0x3d044b19,0x39c556ae,
  0x278206ab,0x23431b1c,0x2e003dc5,0x2ac12072,
  0x128e9dcf,0x164f8078,0x1b0ca6a1,0x1fcdbb16,
  0x018aeb13,0x054bf6a4,0x0808d07d,0x0cc9cdca,
  0x7897ab07,0x7c56b6b0,0x71159069,0x75d48dde,
  0x6b93dddb,0x6f52c06c,0x6211e6b5,0x66d0fb02,
  0x5e9f46bf,0x5a5e5b08,0x571d7dd1,0x53dc6066,
  0x4d9b3063,0x495a2dd4,0x44190b0d,0x40d816ba,
  0xaca5c697,0xa864db20,0xa527fdf9,0xa1e6e04e,
  0xbfa1b04b,0xbb60adfc,0xb6238b25,0xb2e29692,
  0x8aad2b2f,0x8e6c3698,0x832f1041,0x87ee0df6,
  0x99a95df3,0x9d684044,0x902b669d,0x94ea7b2a,
  0xe0b41de7,0xe4750050,0xe9362689,0xedf73b3e,
  0xf3b06b3b,0xf771768c,0xfa325055,0xfef34de2,
  0xc6bcf05f,0xc27dede8,0xcf3ecb31,0xcbffd686,
  0xd5b88683,0xd1799b34,0xdc3abded,0xd8fba05a,
  0x690ce0ee,0x6dcdfd59,0x608edb80,0x644fc637,
  0x7a089632,0x7ec98b85,0x738aad5c,0x774bb0eb,
  0x4f040d56,0x4bc510e1,0x46863638,0x42472b8f,
  0x5c007b8a,0x58c1663d,0x558240e4,0x51435d53,
  0x251d3b9e,0x21dc2629,0x2c9f00f0,0x285e1d47,
  0x36194d42,0x32d850f5,0x3f9b762c,0x3b5a6b9b,
  0x0315d626,0x07d4cb91,0x0a97ed48,0x0e56f0ff,
  0x1011a0fa,0x14d0bd4d,0x19939b94,0x1d528623,
  0xf12f560e,0xf5ee4bb9,0xf8ad6d60,0xfc6c70d7,
  0xe22b20d2,0xe6ea3d65,0xeba91bbc,0xef68060b,
  0xd727bbb6,0xd3e6a601,0xdea580d8,0xda649d6f,
  0xc423cd6a,0xc0e2d0dd,0xcda1f604,0xc960ebb3,
  0xbd3e8d7e,0xb9ff90c9,0xb4bcb610,0xb07daba7,
  0xae3afba2,0xaafbe615,0xa7b8c0cc,0xa379dd7b,
  0x9b3660c6,0x9ff77d71,0x92b45ba8,0x9675461f,
  0x8832161a,0x8cf30bad,0x81b02d74,0x857130c3,
  0x5d8a9099,0x594b8d2e,0x5408abf7,0x50c9b640,
  0x4e8ee645,0x4a4ffbf2,0x470cdd2b,0x43cdc09c,
  0x7b827d21,0x7f436096,0x7200464f,0x76c15bf8,
  0x68860bfd,0x6c47164a,0x61043093,0x65c52d24,
  0x119b4be9,0x155a565e,0x18197087,0x1cd86d30,
  0x029f3d35,0x065e2082,0x0b1d065b,0x0fdc1bec,
  0x3793a651,0x3352bbe6,0x3e119d3f,0x3ad08088,
  0x2497d08d,0x2056cd3a,0x2d15ebe3,0x29d4f654,
  0xc5a92679,0xc1683bce,0xcc2b1d17,0xc8ea00a0,
  0xd6ad50a5,0xd26c4d12,0xdf2f6bcb,0xdbee767c,
  0xe3a1cbc1,0xe760d676,0xea23f0af,0xeee2ed18,
  0xf0a5bd1d,0xf464a0aa,0xf9278673,0xfde69bc4,
  0x89b8fd09,0x8d79e0be,0x803ac667,0x84fbdbd0,
  0x9abc8bd5,0x9e7d9662,0x933eb0bb,0x97ffad0c,
  0xafb010b1,0xab710d06,0xa6322bdf,0xa2f33668,
  0xbcb4666d,0xb8757bda,0xb5365d03,0xb1f740b4};

static void ogg_page_checksum_set(unsigned char *page, oggp_int32 len){
  oggp_uint32 crc_reg=0;
  oggp_int32 i;

  /* safety; needed for API behavior, but not framing code */
  page[22]=0;
  page[23]=0;
  page[24]=0;
  page[25]=0;

  for(i=0;i<len;i++) crc_reg=(crc_reg<<8)^crc_lookup[((crc_reg >> 24)&0xff)^page[i]];

  page[22]=(unsigned char)(crc_reg&0xff);
  page[23]=(unsigned char)((crc_reg>>8)&0xff);
  page[24]=(unsigned char)((crc_reg>>16)&0xff);
  page[25]=(unsigned char)((crc_reg>>24)&0xff);
}

typedef struct {
  oggp_uint64 granulepos;
  size_t buf_pos;
  size_t buf_size;
  size_t lacing_pos;
  size_t lacing_size;
  int flags;
  size_t pageno;
} oggp_page;

struct oggpacker {
  oggp_int32 serialno;
  unsigned char *buf;
  unsigned char *alloc_buf;
  unsigned char *user_buf;
  size_t buf_size;
  size_t buf_fill;
  size_t buf_begin;
  unsigned char *lacing;
  size_t lacing_size;
  size_t lacing_fill;
  size_t lacing_begin;
  oggp_page *pages;
  size_t pages_size;
  size_t pages_fill;
  oggp_uint64 muxing_delay;
  int is_eos;
  oggp_uint64 curr_granule;
  oggp_uint64 last_granule;
  size_t pageno;
};

/** Allocates an oggpacker object */
oggpacker *oggp_create(oggp_int32 serialno) {
  oggpacker *oggp;
  oggp = malloc(sizeof(*oggp));
  if (oggp == NULL) goto fail;
  oggp->alloc_buf = NULL;
  oggp->lacing = NULL;
  oggp->pages = NULL;
  oggp->user_buf = NULL;

  oggp->buf_size = MAX_PAGE_SIZE;
  oggp->lacing_size = 256;
  oggp->pages_size = 10;

  oggp->alloc_buf = malloc(oggp->buf_size + MAX_HEADER_SIZE);
  oggp->lacing = malloc(oggp->lacing_size);
  oggp->pages = malloc(oggp->pages_size * sizeof(oggp->pages[0]));
  if (!oggp->alloc_buf || !oggp->lacing || !oggp->pages) goto fail;
  oggp->buf = oggp->alloc_buf + MAX_HEADER_SIZE;

  oggp->serialno = serialno;
  oggp->buf_fill = 0;
  oggp->buf_begin = 0;
  oggp->lacing_fill = 0;
  oggp->lacing_begin = 0;
  oggp->pages_fill = 0;

  oggp->is_eos = 0;
  oggp->curr_granule = 0;
  oggp->last_granule = 0;
  oggp->pageno = 0;
  oggp->muxing_delay = 0;
  return oggp;
fail:
  if (oggp) {
    if (oggp->lacing) free(oggp->lacing);
    if (oggp->alloc_buf) free(oggp->alloc_buf);
    if (oggp->pages) free(oggp->pages);
    free(oggp);
  }
  return NULL;
}

/** Frees memory associated with an oggpacker object */
void oggp_destroy(oggpacker *oggp) {
  free(oggp->lacing);
  free(oggp->alloc_buf);
  free(oggp->pages);
  free(oggp);
}

/** Sets the maximum muxing delay in granulepos units. Pages will be auto-flushed
    to enforce the delay and to avoid continued pages if possible. */
void oggp_set_muxing_delay(oggpacker *oggp, oggp_uint64 delay) {
  oggp->muxing_delay = delay;
}

static void shift_buffer(oggpacker *oggp) {
  size_t buf_shift;
  size_t lacing_shift;
  size_t i;
  buf_shift = oggp->pages_fill ? oggp->pages[0].buf_pos : oggp->buf_begin;
  lacing_shift = oggp->pages_fill ? oggp->pages[0].lacing_pos : oggp->lacing_begin;
  if (4*lacing_shift > oggp->lacing_fill) {
    memmove(&oggp->lacing[0], &oggp->lacing[lacing_shift], oggp->lacing_fill-lacing_shift);
    for (i=0;i<oggp->pages_fill;i++) oggp->pages[i].lacing_pos -= lacing_shift;
    oggp->lacing_fill -= lacing_shift;
    oggp->lacing_begin -= lacing_shift;
  }
  if (4*buf_shift > oggp->buf_fill) {
    memmove(&oggp->buf[0], &oggp->buf[buf_shift], oggp->buf_fill-buf_shift);
    for (i=0;i<oggp->pages_fill;i++) oggp->pages[i].buf_pos -= buf_shift;
    oggp->buf_fill -= buf_shift;
    oggp->buf_begin -= buf_shift;
  }
}

/** Get a buffer where to write the next packet. The buffer will have
    size "bytes", but fewer bytes can be written. The buffer remains valid through
    a call to oggp_close_page() or oggp_get_next_page(), but is invalidated by
    another call to oggp_get_packet_buffer() or by a call to oggp_commit_packet(). */
unsigned char *oggp_get_packet_buffer(oggpacker *oggp, oggp_int32 bytes) {
  if (oggp->buf_fill + bytes > oggp->buf_size) {
    shift_buffer(oggp);

    /* If we didn't shift the buffer or if we did and there's still not enough room, make some more. */
    if (oggp->buf_fill + bytes > oggp->buf_size) {
      size_t newsize;
      unsigned char *newbuf;
      newsize = oggp->buf_fill + bytes + MAX_HEADER_SIZE;
      /* Making sure we don't need to do that too often. */
      newsize = newsize*3/2;
      newbuf = realloc(oggp->alloc_buf, newsize);
      if (newbuf != NULL) {
        oggp->alloc_buf = newbuf;
        oggp->buf_size = newsize;
        oggp->buf = oggp->alloc_buf + MAX_HEADER_SIZE;
      } else {
        return NULL;
      }
    }
  }
  oggp->user_buf = &oggp->buf[oggp->buf_fill];
  return oggp->user_buf;
}

/** Tells the oggpacker that the packet buffer obtained from
    oggp_get_packet_buffer() has been filled and the number of bytes written
    has to be no more than what was originally asked for. */
int oggp_commit_packet(oggpacker *oggp, oggp_int32 bytes, oggp_uint64 granulepos, int eos) {
  size_t i;
  size_t nb_255s;
  assert(oggp->user_buf != NULL);
  nb_255s = bytes/255;
  if (oggp->lacing_fill-oggp->lacing_begin+nb_255s+1 > 255 ||
      (oggp->muxing_delay && granulepos - oggp->last_granule > oggp->muxing_delay)) {
    oggp_flush_page(oggp);
  }
  assert(oggp->user_buf >= &oggp->buf[oggp->buf_fill]);
  oggp->buf_fill += bytes;
  if (oggp->lacing_fill + nb_255s + 1 > oggp->lacing_size) {
    shift_buffer(oggp);

    /* If we didn't shift the values or if we did and there's still not enough room, make some more. */
    if (oggp->lacing_fill + nb_255s + 1 > oggp->lacing_size) {
      size_t newsize;
      unsigned char *newbuf;
      newsize = oggp->lacing_fill + nb_255s + 1;
      /* Making sure we don't need to do that too often. */
      newsize = newsize*3/2;
      newbuf = realloc(oggp->lacing, newsize);
      if (newbuf != NULL) {
        oggp->lacing = newbuf;
        oggp->lacing_size = newsize;
      } else {
        return 1;
      }
    }
  }
  /* If we moved the buffer data, update the incoming packet location. */
  if (oggp->user_buf > &oggp->buf[oggp->buf_fill]) {
    memmove(&oggp->buf[oggp->buf_fill], oggp->user_buf, bytes);
  }
  for (i=0;i<nb_255s;i++) {
    oggp->lacing[oggp->lacing_fill+i] = 255;
  }
  oggp->lacing[oggp->lacing_fill+nb_255s] = bytes - 255*nb_255s;
  oggp->lacing_fill += nb_255s + 1;
  oggp->curr_granule = granulepos;
  oggp->is_eos = eos;
  if (oggp->muxing_delay && granulepos - oggp->last_granule >= oggp->muxing_delay) {
    oggp_flush_page(oggp);
  }
  return 0;
}

/** Create a page from the data written so far (and not yet part of a previous page).
    If there is too much data for one page, all page continuations will be closed too. */
int oggp_flush_page(oggpacker *oggp) {
  oggp_page *p;
  int cont = 0;
  size_t nb_lacing;
  if (oggp->lacing_fill == oggp->lacing_begin) {
    return 1;
  }
  nb_lacing = oggp->lacing_fill - oggp->lacing_begin;
  do {
    if (oggp->pages_fill >= oggp->pages_size) {
      size_t newsize;
      oggp_page *newbuf;
      /* Making sure we don't need to do that too often. */
      newsize = 1 + oggp->pages_size*3/2;
      newbuf = realloc(oggp->pages, newsize*sizeof(oggp_page));
      assert(newbuf != NULL);
      oggp->pages = newbuf;
      oggp->pages_size = newsize;
    }
    p = &oggp->pages[oggp->pages_fill++];
    p->granulepos = oggp->curr_granule;

    p->lacing_pos = oggp->lacing_begin;
    p->lacing_size = nb_lacing;
    p->flags = cont;
    p->buf_pos = oggp->buf_begin;
    if (p->lacing_size > 255) {
      size_t bytes=0;
      int i;
      for (i=0;i<255;i++) bytes += oggp->lacing[oggp->lacing_begin+1];
      p->buf_size = bytes;
      p->lacing_size = 255;
      p->granulepos = -1;
      cont = 1;
    } else {
      p->buf_size = oggp->buf_fill - oggp->buf_begin;
      if (oggp->is_eos) p->flags |= 0x04;
    }
    nb_lacing -= p->lacing_size;
    oggp->lacing_begin += p->lacing_size;
    oggp->buf_begin += p->buf_size;
    p->pageno = oggp->pageno++;
    if (p->pageno == 0)
      p->flags |= 0x02;
  } while (nb_lacing>0);

  oggp->last_granule = oggp->curr_granule;
  return 0;
}

/** Get a pointer to the contents of the next available page. Pointer is
    invalidated on the next call to oggp_get_next_page() or oggp_commit_packet(). */
int oggp_get_next_page(oggpacker *oggp, unsigned char **page, oggp_int32 *bytes) {
  oggp_page *p;
  int i;
  unsigned char *ptr;
  size_t len;
  int header_size;
  oggp_uint64 granule_pos;
  if (oggp->pages_fill == 0) {
    *page = NULL;
    *bytes = 0;
    return 0;
  }
  p = &oggp->pages[0];
  header_size = 27 + p->lacing_size;
  /* Don't use indexing in case header_size > p->buf_pos. */
  ptr = oggp->buf + p->buf_pos - header_size;
  len = p->buf_size + header_size;
  memcpy(&ptr[27], &oggp->lacing[p->lacing_pos], p->lacing_size);
  memcpy(ptr, "OggS", 4);

  /* stream structure version */
  ptr[4]=0x00;

  ptr[5]=0x00 | p->flags;

  granule_pos = p->granulepos;
  /* 64 bits of PCM position */
  for(i=6;i<14;i++){
    ptr[i]=(unsigned char)(granule_pos&0xff);
    granule_pos>>=8;
  }

  /* 32 bits of stream serial number */
  {
    oggp_int32 serialno=oggp->serialno;
    for(i=14;i<18;i++){
      ptr[i]=(unsigned char)(serialno&0xff);
      serialno>>=8;
    }
  }

  {
    oggp_int32 pageno=p->pageno;
    for(i=18;i<22;i++){
      ptr[i]=(unsigned char)(pageno&0xff);
      pageno>>=8;
    }
  }

  ptr[26] = p->lacing_size;

  /* CRC is always last. */
  ogg_page_checksum_set(ptr, len);

  *page = ptr;
  *bytes = len;
  oggp->pages_fill--;
  memmove(&oggp->pages[0], &oggp->pages[1], oggp->pages_fill*sizeof(oggp_page));
  return 1;
}

/** Creates a new (chained) stream. This closes all outstanding pages. These
    pages remain available with oggp_get_next_page(). */
int oggp_chain(oggpacker *oggp, oggp_int32 serialno) {
  oggp_flush_page(oggp);
  oggp->serialno = serialno;
  oggp->curr_granule = 0;
  oggp->last_granule = 0;
  oggp->is_eos = 0;
  oggp->pageno = 0;
  return 0;
}
