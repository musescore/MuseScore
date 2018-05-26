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

#include "libmscore/xml.h"
#include "audiofile/audiofile.h"
#include "thirdparty/qzip/qzipreader_p.h"

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
      delete[] _data;
      }

//---------------------------------------------------------
//   readSample
//---------------------------------------------------------

Sample* ZInstrument::readSample(const QString& s, MQZipReader* uz)
      {
      if (uz) {
            QList<MQZipReader::FileInfo> fi = uz->fileInfoList();

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

      AudioFile a;
      if (!a.open(buf)) {
            printf("open <%s> failed: %s\n", qPrintable(s), a.error());
            return 0;
            }

      int channel = a.channels();
      sf_count_t frames  = a.frames();
      int sr      = a.samplerate();

      short* data = new short[(frames + 3) * channel];
      Sample* sa  = new Sample(channel, data, frames, sr);
      sa->setLoopStart(a.loopStart());
      sa->setLoopEnd(a.loopEnd());
      sa->setLoopMode(a.loopMode());

      if (frames != a.readData(data + channel, frames)) {
            qDebug("Sample read failed: %s\n", a.error());
            delete sa;
            sa = 0;
            }
      for (int i = 0; i < channel; ++i) {
            data[i]                        = data[channel + i];
            data[(frames-1) * channel + i] = data[(frames-3) * channel + i];
            data[(frames-2) * channel + i] = data[(frames-3) * channel + i];
            }
      return sa;
      }

//---------------------------------------------------------
//   ZInstrument
//---------------------------------------------------------

ZInstrument::ZInstrument(Zerberus* z)
      {
      zerberus  = z;
      for (int i =0; i < 128; i++)
            _setcc[i] = -1;
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
      _name = fi.completeBaseName();
      if (fi.isFile())
            return loadFromFile(path);
      if (fi.isDir())
            return loadFromDir(path);
      qDebug("not file nor dir %s", qPrintable(path));
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
      MQZipReader uz(path);
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

bool ZInstrument::read(const QByteArray& buf, MQZipReader* /*uz*/, const QString& /*path*/)
      {
      Ms::XmlReader e(buf);
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

