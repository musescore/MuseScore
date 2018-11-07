//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __EXPORTMP3_H__
#define __EXPORTMP3_H__

#include "globals.h"
#include "lame/lame.h"
#if defined(Q_OS_WIN)
#include "windows.h"
#endif

namespace Ms {

//----------------------------------------------------------------------------
// ExportMP3Options
//----------------------------------------------------------------------------

#define MODE_SET           0
#define MODE_VBR           1
#define MODE_ABR           2
#define MODE_CBR           3

#define CHANNEL_JOINT      0
#define CHANNEL_STEREO     1

#define QUALITY_0          0
#define QUALITY_1          1
#define QUALITY_2          2
#define QUALITY_3          3
#define QUALITY_4          4
#define QUALITY_5          5
#define QUALITY_6          6
#define QUALITY_7          7
#define QUALITY_8          8
#define QUALITY_9          9

#define ROUTINE_FAST       0
#define ROUTINE_STANDARD   1

#define PRESET_INSANE      0
#define PRESET_EXTREME     1
#define PRESET_STANDARD    2
#define PRESET_MEDIUM      3


//----------------------------------------------------------------------------
// MP3Exporter
//----------------------------------------------------------------------------

typedef lame_global_flags *lame_init_t(void);
typedef int lame_init_params_t(lame_global_flags*);
typedef const char* get_lame_version_t(void);

typedef int lame_encode_buffer_float_t (
      lame_global_flags* gf,
      const float        buffer_l [],
      const float        buffer_r [],
      const int          nsamples,
      unsigned char *    mp3buf,
      const int          mp3buf_size );

typedef int lame_encode_flush_t(
      lame_global_flags *gf,
      unsigned char*     mp3buf,
      int                size );

typedef int lame_close_t(lame_global_flags*);

typedef int lame_set_in_samplerate_t(lame_global_flags*, int);
typedef int lame_set_out_samplerate_t(lame_global_flags*, int);
typedef int lame_set_num_channels_t(lame_global_flags*, int );
typedef int lame_set_quality_t(lame_global_flags*, int);
typedef int lame_set_brate_t(lame_global_flags*, int);
typedef int lame_set_VBR_t(lame_global_flags *, vbr_mode);
typedef int lame_set_VBR_q_t(lame_global_flags *, int);
typedef int lame_set_VBR_min_bitrate_kbps_t(lame_global_flags *, int);
typedef int lame_set_mode_t(lame_global_flags *, MPEG_mode);
typedef int lame_set_preset_t(lame_global_flags *, int);
typedef int lame_set_error_protection_t(lame_global_flags *, int);
typedef int lame_set_disable_reservoir_t(lame_global_flags *, int);
typedef int lame_set_padding_type_t(lame_global_flags *, Padding_type);
typedef int lame_set_bWriteVbrTag_t(lame_global_flags *, int);
typedef size_t lame_get_lametag_frame_t(const lame_global_flags *, unsigned char* buffer, size_t size);
typedef void lame_mp3_tags_fid_t(lame_global_flags *, FILE *);

#if defined(Q_OS_WIN)
// An alternative solution to give Windows an additional chance of writing the tag before
// falling bato to lame_mp3_tag_fid().  The latter can have DLL sharing issues when mixing
// Debug/Release builds of Audacity and the lame DLL.
typedef unsigned long beWriteInfoTag_t(lame_global_flags *, char *);

// We use this to determine if the user has selected an older, Blade API only, lame_enc.dll
// so we can be more specific about why their library isn't acceptable.
typedef struct  {

      // BladeEnc DLL Version number

      BYTE    byDLLMajorVersion;
      BYTE    byDLLMinorVersion;

      // BladeEnc Engine Version Number

      BYTE    byMajorVersion;
      BYTE    byMinorVersion;

      // DLL Release date

      BYTE    byDay;
      BYTE    byMonth;
      WORD    wYear;

      // BladeEnc     Homepage URL

      CHAR    zHomepage[129];

      BYTE    byAlphaLevel;
      BYTE    byBetaLevel;
      BYTE    byMMXEnabled;

      BYTE    btReserved[125];
} be_version;
typedef void beVersion_t(be_version *);
#endif

//---------------------------------------------------------
//   MP3Exporter
//---------------------------------------------------------

class MP3Exporter {

   public:
      enum class AskUser : char { NO, MAYBE, YES };

      MP3Exporter();
      virtual ~MP3Exporter();

      bool findLibrary();
      bool loadLibrary(AskUser askuser);
      bool validLibraryLoaded();

      /* These global settings keep state over the life of the object */
      void setMode(int mode);
      void setBitrate(int rate);
      void setQuality(int q, int r);
      void setChannel(int mode);

      /* Virtual methods that must be supplied by library interfaces */

      /* initialize the library interface */
      bool initLibrary(QString libpath);
      void freeLibrary();

      /* get library info */
      QString getLibraryVersion();
      QString getLibraryName();
      QString getLibraryPath();
      QString getLibraryTypeString();

      /* returns the number of samples PER CHANNEL to send for each call to EncodeBuffer */
      int initializeStream(int channels, int sampleRate);

      /* In bytes. must be called AFTER InitializeStream */
      int getOutBufferSize();

      /* returns the number of bytes written. input is separate per channel */
      void bufferPreamp(float buffer[], int nSamples);
      int encodeBuffer(float inbufferL[], float inbufferR[], unsigned char outbuffer[]);
      int encodeRemainder(float inbufferL[], float inbufferR[], int nSamples,
         unsigned char outbuffer[]);

      int encodeBufferMono(float inbuffer[], unsigned char outbuffer[]);
      int encodeRemainderMono(float inbuffer[], int nSamples,
                           unsigned char outbuffer[]);

      int finishStream(unsigned char outbuffer[]);
      void cancelEncoding();

//   void PutInfoTag(QFile f, qint64 off);

   private:
      QString mLibPath;
      QLibrary* lame_lib;
      bool mLibraryLoaded;

#if defined(Q_OS_WIN)
      QString mBladeVersion;
#endif

      bool mEncoding;
      int mMode;
      int mBitrate;
      int mQuality;
      int mRoutine;
      int mChannel;

      /* function pointers to the symbols we get from the library */
      lame_init_t* lame_init;
      lame_init_params_t* lame_init_params;
      lame_encode_buffer_float_t* lame_encode_buffer_float;
      lame_encode_flush_t* lame_encode_flush;
      lame_close_t* lame_close;
      get_lame_version_t* get_lame_version;

      lame_set_in_samplerate_t* lame_set_in_samplerate;
      lame_set_out_samplerate_t* lame_set_out_samplerate;
      lame_set_num_channels_t* lame_set_num_channels;
      lame_set_quality_t* lame_set_quality;
      lame_set_brate_t* lame_set_brate;
      lame_set_VBR_t* lame_set_VBR;
      lame_set_VBR_q_t* lame_set_VBR_q;
      lame_set_VBR_min_bitrate_kbps_t* lame_set_VBR_min_bitrate_kbps;
      lame_set_mode_t* lame_set_mode;
      lame_set_preset_t* lame_set_preset;
      lame_set_error_protection_t* lame_set_error_protection;
      lame_set_disable_reservoir_t *lame_set_disable_reservoir;
      lame_set_padding_type_t *lame_set_padding_type;
      lame_set_bWriteVbrTag_t *lame_set_bWriteVbrTag;
      lame_get_lametag_frame_t *lame_get_lametag_frame;
      lame_mp3_tags_fid_t *lame_mp3_tags_fid;
#if defined(Q_OS_WIN)
      beWriteInfoTag_t *beWriteInfoTag;
      beVersion_t *beVersion;
#endif

      lame_global_flags *mGF;

      static const int mSamplesPerChunk = 220500;
      // See lame.h/lame_encode_buffer() for further explanation
      // As coded here, this should be the worst case.
      static const int mOutBufferSize =
         mSamplesPerChunk * (320 / 8) / 8 + 4 * 1152 * (320 / 8) / 8 + 512;

      // See MAXFRAMESIZE in libmp3lame/VbrTag.c for explanation of 2880.
      unsigned char mInfoTagBuf[2880];
      size_t mInfoTagLen;
      };


} // namespace Ms
#endif //__EXPORTMP3_H__
