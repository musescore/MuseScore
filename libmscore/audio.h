//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __AUDIO_H__
#define __AUDIO_H__

namespace Ms {

class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   Audio
//---------------------------------------------------------

class Audio {
      QString _path;
      QByteArray _data;

   public:
      Audio();
      const QString& path() const        { return _path; }
      void setPath(const QString& s)     { _path = s;    }
      const QByteArray& data() const     { return _data; }
      QByteArray data()                  { return _data; }
      void setData(const QByteArray& ba) { _data = ba;   }

      void read(XmlReader&);
      void write(XmlWriter&) const;
      };


}     // namespace Ms
#endif

