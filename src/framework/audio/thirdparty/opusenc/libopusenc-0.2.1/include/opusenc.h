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

#ifndef OPUSENC_H
# define OPUSENC_H

/**\mainpage
   \section Introduction

   This is the documentation for the <tt>libopusenc</tt> C API.

   The <tt>libopusenc</tt> package provides a convenient high-level API for
   encoding Ogg Opus files.

   \section Organization

   The main API is divided into several sections:
   - \ref encoding
   - \ref comments
   - \ref encoder_ctl
   - \ref callbacks
   - \ref error_codes

   \section Overview

   The <tt>libopusfile</tt> API provides an easy way to encode Ogg Opus files using
   <tt>libopus</tt>.
*/

# if defined(__cplusplus)
extern "C" {
# endif

#include <stddef.h>
#include <opus.h>

#ifndef OPE_EXPORT
# if defined(WIN32)
#  if defined(OPE_BUILD) && defined(DLL_EXPORT)
#   define OPE_EXPORT __declspec(dllexport)
#  else
#   define OPE_EXPORT
#  endif
# elif defined(__GNUC__) && defined(OPE_BUILD)
#  define OPE_EXPORT __attribute__ ((visibility ("default")))
# else
#  define OPE_EXPORT
# endif
#endif

/**\defgroup error_codes Error Codes*/
/*@{*/
/**\name List of possible error codes
   Many of the functions in this library return a negative error code when a
    function fails.
   This list provides a brief explanation of the common errors.
   See each individual function for more details on what a specific error code
    means in that context.*/
/*@{*/


/* Bump this when we change the API. */
/** API version for this header. Can be used to check for features at compile time. */
#define OPE_API_VERSION 0

#define OPE_OK 0
/* Based on the relevant libopus code minus 10. */
#define OPE_BAD_ARG -11
#define OPE_INTERNAL_ERROR -13
#define OPE_UNIMPLEMENTED -15
#define OPE_ALLOC_FAIL -17

/* Specific to libopusenc. */
#define OPE_CANNOT_OPEN -30
#define OPE_TOO_LATE -31
#define OPE_INVALID_PICTURE -32
#define OPE_INVALID_ICON -33
#define OPE_WRITE_FAIL -34
#define OPE_CLOSE_FAIL -35

/*@}*/
/*@}*/


/* These are the "raw" request values -- they should usually not be used. */
#define OPE_SET_DECISION_DELAY_REQUEST      14000
#define OPE_GET_DECISION_DELAY_REQUEST      14001
#define OPE_SET_MUXING_DELAY_REQUEST        14002
#define OPE_GET_MUXING_DELAY_REQUEST        14003
#define OPE_SET_COMMENT_PADDING_REQUEST     14004
#define OPE_GET_COMMENT_PADDING_REQUEST     14005
#define OPE_SET_SERIALNO_REQUEST            14006
#define OPE_GET_SERIALNO_REQUEST            14007
#define OPE_SET_PACKET_CALLBACK_REQUEST     14008
/*#define OPE_GET_PACKET_CALLBACK_REQUEST     14009*/
#define OPE_SET_HEADER_GAIN_REQUEST         14010
#define OPE_GET_HEADER_GAIN_REQUEST         14011
#define OPE_GET_NB_STREAMS_REQUEST          14013
#define OPE_GET_NB_COUPLED_STREAMS_REQUEST  14015

/**\defgroup encoder_ctl Encoding Options*/
/*@{*/

/**\name Control parameters

   Macros for setting encoder options.*/
/*@{*/

#define OPE_SET_DECISION_DELAY(x) OPE_SET_DECISION_DELAY_REQUEST, __opus_check_int(x)
#define OPE_GET_DECISION_DELAY(x) OPE_GET_DECISION_DELAY_REQUEST, __opus_check_int_ptr(x)
#define OPE_SET_MUXING_DELAY(x) OPE_SET_MUXING_DELAY_REQUEST, __opus_check_int(x)
#define OPE_GET_MUXING_DELAY(x) OPE_GET_MUXING_DELAY_REQUEST, __opus_check_int_ptr(x)
#define OPE_SET_COMMENT_PADDING(x) OPE_SET_COMMENT_PADDING_REQUEST, __opus_check_int(x)
#define OPE_GET_COMMENT_PADDING(x) OPE_GET_COMMENT_PADDING_REQUEST, __opus_check_int_ptr(x)
#define OPE_SET_SERIALNO(x) OPE_SET_SERIALNO_REQUEST, __opus_check_int(x)
#define OPE_GET_SERIALNO(x) OPE_GET_SERIALNO_REQUEST, __opus_check_int_ptr(x)
/* FIXME: Add type-checking macros to these. */
#define OPE_SET_PACKET_CALLBACK(x,u) OPE_SET_PACKET_CALLBACK_REQUEST, (x), (u)
/*#define OPE_GET_PACKET_CALLBACK(x,u) OPE_GET_PACKET_CALLBACK_REQUEST, (x), (u)*/
#define OPE_SET_HEADER_GAIN(x) OPE_SET_HEADER_GAIN_REQUEST, __opus_check_int(x)
#define OPE_GET_HEADER_GAIN(x) OPE_GET_HEADER_GAIN_REQUEST, __opus_check_int_ptr(x)
#define OPE_GET_NB_STREAMS(x) OPE_GET_NB_STREAMS_REQUEST, __opus_check_int_ptr(x)
#define OPE_GET_NB_COUPLED_STREAMS(x) OPE_GET_NB_COUPLED_STREAMS_REQUEST, __opus_check_int_ptr(x)
/*@}*/
/*@}*/

/**\defgroup callbacks Callback Functions */
/*@{*/

/**\name Callback functions

   These are the callbacks that can be implemented for an encoder.*/
/*@{*/

/** Called for writing a page.
 \param user_data user-defined data passed to the callback
 \param ptr       buffer to be written
 \param len       number of bytes to be written
 \return          error code
 \retval 0        success
 \retval 1        failure
 */
typedef int (*ope_write_func)(void *user_data, const unsigned char *ptr, opus_int32 len);

/** Called for closing a stream.
 \param user_data user-defined data passed to the callback
 \return          error code
 \retval 0        success
 \retval 1        failure
 */
typedef int (*ope_close_func)(void *user_data);

/** Called on every packet encoded (including header).
 \param user_data   user-defined data passed to the callback
 \param packet_ptr  packet data
 \param packet_len  number of bytes in the packet
 \param flags       optional flags (none defined for now so zero)
 */
typedef void (*ope_packet_func)(void *user_data, const unsigned char *packet_ptr, opus_int32 packet_len, opus_uint32 flags);

/** Callback functions for accessing the stream. */
typedef struct {
  /** Callback for writing to the stream. */
  ope_write_func write;
  /** Callback for closing the stream. */
  ope_close_func close;
} OpusEncCallbacks;
/*@}*/
/*@}*/

/** Opaque comments struct. */
typedef struct OggOpusComments OggOpusComments;

/** Opaque encoder struct. */
typedef struct OggOpusEnc OggOpusEnc;

/**\defgroup comments Comments Handling */
/*@{*/

/**\name Functions for handling comments

   These functions make it possible to add comments and pictures to Ogg Opus files.*/
/*@{*/

/** Create a new comments object.
    \return Newly-created comments object. */
OPE_EXPORT OggOpusComments *ope_comments_create(void);

/** Create a deep copy of a comments object.
    \param comments Comments object to copy
    \return Deep copy of input. */
OPE_EXPORT OggOpusComments *ope_comments_copy(OggOpusComments *comments);

/** Destroys a comments object.
    \param comments Comments object to destroy*/
OPE_EXPORT void ope_comments_destroy(OggOpusComments *comments);

/** Add a comment.
    \param[in,out] comments Where to add the comments
    \param         tag      Tag for the comment (must not contain = char)
    \param         val      Value for the tag
    \return Error code
 */
OPE_EXPORT int ope_comments_add(OggOpusComments *comments, const char *tag, const char *val);

/** Add a comment as a single tag=value string.
    \param[in,out] comments    Where to add the comments
    \param         tag_and_val string of the form tag=value (must contain = char)
    \return Error code
 */
OPE_EXPORT int ope_comments_add_string(OggOpusComments *comments, const char *tag_and_val);

/** Add a picture from a file.
    \param[in,out] comments     Where to add the comments
    \param         filename     File name for the picture
    \param         picture_type Type of picture (-1 for default)
    \param         description  Description (NULL means no comment)
    \return Error code
 */
OPE_EXPORT int ope_comments_add_picture(OggOpusComments *comments, const char *filename, int picture_type, const char *description);

/** Add a picture already in memory.
    \param[in,out] comments     Where to add the comments
    \param         ptr          Pointer to picture in memory
    \param         size         Size of picture pointed to by ptr
    \param         picture_type Type of picture (-1 for default)
    \param         description  Description (NULL means no comment)
    \return Error code
 */
OPE_EXPORT int ope_comments_add_picture_from_memory(OggOpusComments *comments, const char *ptr, size_t size, int picture_type, const char *description);

/*@}*/
/*@}*/

/**\defgroup encoding Encoding */
/*@{*/

/**\name Functions for encoding Ogg Opus files

   These functions make it possible to encode Ogg Opus files.*/
/*@{*/

/** Create a new OggOpus file.
    \param path       Path where to create the file
    \param comments   Comments associated with the stream
    \param rate       Input sampling rate (48 kHz is faster)
    \param channels   Number of channels
    \param family     Mapping family (0 for mono/stereo, 1 for surround)
    \param[out] error Error code (NULL if no error is to be returned)
    \return Newly-created encoder.
    */
OPE_EXPORT OggOpusEnc *ope_encoder_create_file(const char *path, OggOpusComments *comments, opus_int32 rate, int channels, int family, int *error);

/** Create a new OggOpus stream to be handled using callbacks
    \param callbacks  Callback functions
    \param user_data  Pointer to be associated with the stream and passed to the callbacks
    \param comments   Comments associated with the stream
    \param rate       Input sampling rate (48 kHz is faster)
    \param channels   Number of channels
    \param family     Mapping family (0 for mono/stereo, 1 for surround)
    \param[out] error Error code (NULL if no error is to be returned)
    \return Newly-created encoder.
    */
OPE_EXPORT OggOpusEnc *ope_encoder_create_callbacks(const OpusEncCallbacks *callbacks, void *user_data,
    OggOpusComments *comments, opus_int32 rate, int channels, int family, int *error);

/** Create a new OggOpus stream to be used along with.ope_encoder_get_page().
  This is mostly useful for muxing with other streams.
    \param comments   Comments associated with the stream
    \param rate       Input sampling rate (48 kHz is faster)
    \param channels   Number of channels
    \param family     Mapping family (0 for mono/stereo, 1 for surround)
    \param[out] error Error code (NULL if no error is to be returned)
    \return Newly-created encoder.
    */
OPE_EXPORT OggOpusEnc *ope_encoder_create_pull(OggOpusComments *comments, opus_int32 rate, int channels, int family, int *error);

/** Deferred initialization of the encoder to force an explicit channel mapping. This can be used to override the default channel coupling,
    but using it for regular surround will almost certainly lead to worse quality.
    \param[in,out] enc         Encoder
    \param family              Mapping family (0 for mono/stereo, 1 for surround)
    \param streams             Total number of streams
    \param coupled_streams     Number of coupled streams
    \param mapping             Channel mapping
    \return Error code
 */
OPE_EXPORT int ope_encoder_deferred_init_with_mapping(OggOpusEnc *enc, int family, int streams,
    int coupled_streams, const unsigned char *mapping);

/** Add/encode any number of float samples to the stream.
    \param[in,out] enc         Encoder
    \param pcm                 Floating-point PCM values in the +/-1 range (interleaved if multiple channels)
    \param samples_per_channel Number of samples for each channel
    \return Error code*/
OPE_EXPORT int ope_encoder_write_float(OggOpusEnc *enc, const float *pcm, int samples_per_channel);

/** Add/encode any number of 16-bit linear samples to the stream.
    \param[in,out] enc         Encoder
    \param pcm                 Linear 16-bit PCM values in the [-32768,32767] range (interleaved if multiple channels)
    \param samples_per_channel Number of samples for each channel
    \return Error code*/
OPE_EXPORT int ope_encoder_write(OggOpusEnc *enc, const opus_int16 *pcm, int samples_per_channel);

/** Get the next page from the stream (only if using ope_encoder_create_pull()).
    \param[in,out] enc Encoder
    \param[out] page   Next available encoded page
    \param[out] len    Size (in bytes) of the page returned
    \param flush       If non-zero, forces a flush of the page (if any data avaiable)
    \return 1 if there is a page available, 0 if not. */
OPE_EXPORT int ope_encoder_get_page(OggOpusEnc *enc, unsigned char **page, opus_int32 *len, int flush);

/** Finalizes the stream, but does not deallocate the object.
    \param[in,out] enc Encoder
    \return Error code
 */
OPE_EXPORT int ope_encoder_drain(OggOpusEnc *enc);

/** Deallocates the obect. Make sure to ope_drain() first.
    \param[in,out] enc Encoder
 */
OPE_EXPORT void ope_encoder_destroy(OggOpusEnc *enc);

/** Ends the stream and create a new stream within the same file.
    \param[in,out] enc Encoder
    \param comments   Comments associated with the stream
    \return Error code
 */
OPE_EXPORT int ope_encoder_chain_current(OggOpusEnc *enc, OggOpusComments *comments);

/** Ends the stream and create a new file.
    \param[in,out] enc Encoder
    \param path        Path where to write the new file
    \param comments    Comments associated with the stream
    \return Error code
 */
OPE_EXPORT int ope_encoder_continue_new_file(OggOpusEnc *enc, const char *path, OggOpusComments *comments);

/** Ends the stream and create a new file (callback-based).
    \param[in,out] enc Encoder
    \param user_data   Pointer to be associated with the new stream and passed to the callbacks
    \param comments    Comments associated with the stream
    \return Error code
 */
OPE_EXPORT int ope_encoder_continue_new_callbacks(OggOpusEnc *enc, void *user_data, OggOpusComments *comments);

/** Write out the header now rather than wait for audio to begin.
    \param[in,out] enc Encoder
    \return Error code
 */
OPE_EXPORT int ope_encoder_flush_header(OggOpusEnc *enc);

/** Sets encoder options.
    \param[in,out] enc Encoder
    \param request     Use a request macro
    \return Error code
 */
OPE_EXPORT int ope_encoder_ctl(OggOpusEnc *enc, int request, ...);

/** Converts a libopusenc error code into a human readable string.
  *
  * @param error Error number
  * @returns Error string
  */
OPE_EXPORT const char *ope_strerror(int error);

/** Returns a string representing the version of libopusenc being used at run time.
    \return A string describing the version of this library */
OPE_EXPORT const char *ope_get_version_string(void);

/** ABI version for this header. Can be used to check for features at run time.
    \return An integer representing the ABI version */
OPE_EXPORT int ope_get_abi_version(void);

/*@}*/
/*@}*/

# if defined(__cplusplus)
}
# endif

#endif
