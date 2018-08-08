//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2007-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "audiofile.h"

static sf_count_t getFileLen(void* f) {
      return ((AudioFile*)f)->getFileLen();
      }
static sf_count_t seek(sf_count_t offset, int whence, void* f) {
      return ((AudioFile*)f)->seek(offset, whence);
      }
static sf_count_t read(void* ptr, sf_count_t count, void* f) {
      return ((AudioFile*)f)->read(ptr, count);
      }
static sf_count_t write(const void* ptr, sf_count_t count, void* f) {
      return ((AudioFile*)f)->write(ptr, count);
      }
static sf_count_t tell(void* f) {
      return ((AudioFile*)f)->tell();
      }

static SF_VIRTUAL_IO sfio = {
      getFileLen,
      seek,
      read,
      write,
      tell
      };

//---------------------------------------------------------
//   AudioFile
//---------------------------------------------------------

AudioFile::AudioFile()
      {
      memset(&info, 0, sizeof(info));
      memset(&inst, 0, sizeof(inst));
      sf = 0;
      }

AudioFile::~AudioFile()
      {
      if (sf)
            sf_close(sf);
      }

//---------------------------------------------------------
//   open
//---------------------------------------------------------

bool AudioFile::open(const QByteArray& b)
      {
      buf = b;
      idx = 0;
      sf  = sf_open_virtual(&sfio, SFM_READ, &info, this);
      hasInstrument = sf_command(sf, SFC_GET_INSTRUMENT, &inst, sizeof(inst)) == SF_TRUE;
      _type = info.format & SF_FORMAT_OGG ? fltp : s16p;
      return sf != 0;
      }

//---------------------------------------------------------
//   readData
//---------------------------------------------------------

sf_count_t AudioFile::readData(short* data, sf_count_t frames)
      {
      //see https://musescore.org/en/node/22086#comment-83671
      //see https://github.com/erikd/libsndfile/issues/16
      //this code fixes the bug in libsndfile: float values are not normalized when reading .ogg
      //this leads to overflowing signed short values, reverting a sign and clicking noise
      sf_count_t resFrames = 0;
      if (s16p == _type)
            resFrames = sf_readf_short(sf, data, frames);
      else {
            //read native float values
            int totalFrames = frames * channels();
            float* dataF = new float[totalFrames];
            resFrames = sf_readf_float(sf, dataF, frames);
            //find the maximum signal value
            float maxSignal = 0.f;
            for (int i = 0; i < totalFrames; ++i) {
                  if (fabs(dataF[i]) > maxSignal)
                        maxSignal = dataF[i] > 0 ? dataF[i] : -dataF[i];
                  }
            //normalize values if and only if sample values range is incorrect
            //which means having at least one sample value more than 1.0
            float adjScale = maxSignal > 1.f ? 1.f/maxSignal : 1.f;
            //convert normalized floats to signed short values
            for (int i = 0; i < totalFrames; ++i)
                  data[i] = adjScale * lrintf(dataF[i] * (dataF[i] > 0 ? SHRT_MAX : -SHRT_MIN));
            }

      return resFrames;
      }

//---------------------------------------------------------
//   seek
//---------------------------------------------------------

sf_count_t AudioFile::seek(sf_count_t offset, int whence)
      {
      switch(whence) {
            case SEEK_SET:
                  idx = offset;
                  break;
            case SEEK_CUR:
                  idx += offset;
                  break;
            case SEEK_END:
                  idx = buf.size() + offset;
                  break;
            }
      return idx;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

sf_count_t AudioFile::read(void* ptr, sf_count_t count)
      {
      count = qMin(count, (sf_count_t)(buf.size() - idx));
      memcpy(ptr, buf.data() + idx, count);
      idx += count;
      return count;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

sf_count_t AudioFile::write(const void* /*ptr*/, sf_count_t /*count*/)
      {
      printf("write\n");
      return 0;
      }

