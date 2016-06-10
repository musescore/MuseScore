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

      return sf != 0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

int AudioFile::read(short* data, int frames)
      {
      return sf_readf_short(sf, data, frames);
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

