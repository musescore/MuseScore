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

#ifndef __MINSTRUMENT_H__
#define __MINSTRUMENT_H__

// #include <stdio.h>
#include <list>

#include <QString>

class Zerberus;
class XmlReader;
class QZipReader;
class Zone;
class SfzRegion;
class Sample;

//---------------------------------------------------------
//   ZInstrument
//---------------------------------------------------------

class ZInstrument {
      Zerberus* _msynth;

      QString _name;
      int _program;
      QString instrumentPath;
      std::list<Zone*> _zones;

      bool loadFromFile(const QString&);
      bool loadSfz(const QString&);
      bool loadFromDir(const QString&);
      bool read(const QByteArray&, QZipReader*, const QString& path);

   public:
      ZInstrument(Zerberus*);
      ~ZInstrument();

      bool load(const QString&);
      int program() const                   { return _program; }
      QString name() const                  { return _name;   }
      QString path() const                  { return instrumentPath; }
      const std::list<Zone*>& zones() const { return _zones;  }
      std::list<Zone*>& zones()             { return _zones;  }
      Sample* readSample(const QString& s, QZipReader* uz);
      void addZone(Zone* z)                 { _zones.push_back(z); }
      void addRegion(SfzRegion&);

      static QByteArray buf;  // used during read of Sample
      static int idx;
      };

#endif

