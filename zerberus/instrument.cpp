//=============================================================================
//  Zerberus
//  Zample player
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <stdio.h>
#include <math.h>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <sndfile.h>

#include "libmscore/xml.h"
#include "libmscore/qzipreader_p.h"

#include "instrument.h"
#include "zone.h"
#include "sample.h"

QByteArray ZInstrument::buf;
int ZInstrument::idx;

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

Sample::~Sample()
      {
      delete _data;
      }

//---------------------------------------------------------
//   getFileLen
//---------------------------------------------------------

static sf_count_t getFileLen(void*)
      {
      return ZInstrument::buf.size();
      }

//---------------------------------------------------------
//   seek
//---------------------------------------------------------

static sf_count_t seek(sf_count_t offset, int whence, void*)
      {
      switch(whence) {
            case SEEK_SET:
                  ZInstrument::idx = offset;
                  break;
            case SEEK_CUR:
                  ZInstrument::idx += offset;
                  break;
            case SEEK_END:
                  ZInstrument::idx = ZInstrument::buf.size() + offset;
                  break;
            }
      return ZInstrument::idx;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

static sf_count_t read(void* ptr, sf_count_t count, void*)
      {
//      printf("read %ld at %d = %ld %d\n", count, Instrument::idx, count + Instrument::idx,
//         Instrument::buf.size());
      count = qMin(count, (sf_count_t)(ZInstrument::buf.size() - ZInstrument::idx));
      memcpy(ptr, ZInstrument::buf.data() + ZInstrument::idx, count);
      ZInstrument::idx += count;
      return count;
      }

static sf_count_t write(const void* /*ptr*/, sf_count_t /*count*/, void*)
      {
      printf("write\n");
      return 0;
      }

//---------------------------------------------------------
//   tell
//---------------------------------------------------------

static sf_count_t tell(void*)
      {
      return ZInstrument::idx;
      }

static SF_VIRTUAL_IO sfio = {
      getFileLen,
	seek,
	read,
	write,
	tell
      };

//---------------------------------------------------------
//   readSample
//---------------------------------------------------------

Sample* ZInstrument::readSample(const QString& s, QZipReader* uz)
      {
      if (uz) {
            QList<QZipReader::FileInfo> fi = uz->fileInfoList();

            buf = uz->fileData(s);
            if (buf.isEmpty()) {
                  printf("Sample::read: cannot read sample data <%s>\n", qPrintable(s));
                  return 0;
                  }
            }
      else {
            QFile f(s);
            if (!f.open(QIODevice::ReadOnly)) {
                  printf("Sample::read: open <%s> failed\n", qPrintable(s));
                  return 0;
                  }
            buf = f.readAll();
            }
      SF_INFO info;
      memset(&info, 0, sizeof(info));
      idx = 0;
      SNDFILE* sf = sf_open_virtual(&sfio, SFM_READ, &info, this);
      if (sf == 0) {
            printf("open <%s> failed: %s\n", qPrintable(s), sf_strerror(0));
            return 0;
            }
      short* data = new short[(info.frames + 3) * info.channels];
      int channel = info.channels;
      int frames  = info.frames;
      int sr      = info.samplerate;
      Sample* sa  = new Sample(channel, data, frames, sr);

      if (info.frames != sf_readf_short(sf, data + channel, frames)) {
            printf("Sample read failed: %s\n", sf_strerror(sf));
            delete[] data;
            delete sa;
            sa = 0;
            }
      for (int i = 0; i < channel; ++i) {
            data[i]                        = data[channel + i];
            data[(frames-1) * channel + i] = data[(frames-3) * channel + i];
            data[(frames-2) * channel + i] = data[(frames-3) * channel + i];
            }
      sf_close(sf);
      return sa;
      }

//---------------------------------------------------------
//   ZInstrument
//---------------------------------------------------------

ZInstrument::ZInstrument()
      {
      _program  = -1;
      _refCount = 0;
      }

//---------------------------------------------------------
//   ZInstrument
//---------------------------------------------------------

ZInstrument::~ZInstrument()
      {
      for (Zone* z : _zones)
            delete z;
      }

//---------------------------------------------------------
//   load
//    return true on success
//---------------------------------------------------------

bool ZInstrument::load(const QString& path)
      {
      instrumentPath = path;
      QFileInfo fi(path);
      _name = fi.baseName();
      if (fi.isFile())
            return loadFromFile(path);
      if (fi.isDir())
            return loadFromDir(path);
      return false;
      }

//---------------------------------------------------------
//   loadFromDir
//---------------------------------------------------------

bool ZInstrument::loadFromDir(const QString& s)
      {
      QFile f(s + "/orchestra.xml");
      if (!f.open(QIODevice::ReadOnly)) {
            printf("cannot load orchestra.xml in <%s>\n", qPrintable(s));
            return false;
            }
      QByteArray buf = f.readAll();
      if (buf.isEmpty()) {
            printf("Instrument::loadFromFile: orchestra.xml is empty\n");
            return false;
            }
      return read(buf, 0, s);
      }

//---------------------------------------------------------
//   loadFromFile
//---------------------------------------------------------

bool ZInstrument::loadFromFile(const QString& path)
      {
      if (path.endsWith(".sfz"))
            return loadSfz(path);
      if (!path.endsWith(".msoz")) {
            printf("<%s> not a orchestra file\n", qPrintable(path));
            return false;
            }
      QZipReader uz(path);
      if (!uz.exists()) {
            printf("Instrument::load: %s not found\n", qPrintable(path));
            return false;
            }
      QByteArray buf = uz.fileData("orchestra.xml");
      if (buf.isEmpty()) {
            printf("Instrument::loadFromFile: orchestra.xml not found\n");
            return false;
            }
      return read(buf, &uz, QString());
      }

//---------------------------------------------------------
//   read
//    read orchestra
//---------------------------------------------------------

bool ZInstrument::read(const QByteArray& buf, QZipReader* /*uz*/, const QString& /*path*/)
      {
      XmlReader e(buf);
      while (e.readNextStartElement()) {
            if (e.name() == "MuseSynth") {
                  while (e.readNextStartElement()) {
                        if (e.name() == "Instrument") {
                              // if (!read(e, uz, path))
                              //      return false;
                              }
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      return true;
      }

