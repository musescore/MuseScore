/* Copyright (c) 2017 Jean-Marc Valin */
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

#ifndef OGGPACKER_H
# define OGGPACKER_H


# if defined(__cplusplus)
extern "C" {
# endif

typedef unsigned long long oggp_uint64;
typedef unsigned oggp_uint32;
typedef int oggp_int32;

typedef struct oggpacker oggpacker;

/** Allocates an oggpacker object */
oggpacker *oggp_create(oggp_int32 serialno);

/** Frees memory associated with an oggpacker object */
void oggp_destroy(oggpacker *oggp);

/** Sets the maximum muxing delay in granulepos units. Pages will be auto-flushed
    to enforce the delay and to avoid continued pages if possible. */
void oggp_set_muxing_delay(oggpacker *oggp, oggp_uint64 delay);

/** Get a buffer where to write the next packet. The buffer will have
    size "bytes", but fewer bytes can be written. The buffer remains valid through
    a call to oggp_close_page() or oggp_get_next_page(), but is invalidated by
    another call to oggp_get_packet_buffer() or by a call to oggp_commit_packet(). */
unsigned char *oggp_get_packet_buffer(oggpacker *oggp, oggp_int32 bytes);

/** Tells the oggpacker that the packet buffer obtained from
    oggp_get_packet_buffer() has been filled and the number of bytes written
    has to be no more than what was originally asked for. */
int oggp_commit_packet(oggpacker *oggp, oggp_int32 bytes, oggp_uint64 granulepos, int eos);

/** Create a page from the data written so far (and not yet part of a previous page).
    If there is too much data for one page, then all page continuations will be closed too. */
int oggp_flush_page(oggpacker *oggp);

/** Get a pointer to the contents of the next available page. Pointer is
    invalidated on the next call to oggp_get_next_page() or oggp_commit_packet(). */
int oggp_get_next_page(oggpacker *oggp, unsigned char **page, oggp_int32 *bytes);

/** Creates a new (chained) stream. This closes all outstanding pages. These
    pages remain available with oggp_get_next_page(). */
int oggp_chain(oggpacker *oggp, oggp_int32 serialno);

# if defined(__cplusplus)
}
# endif

#endif
