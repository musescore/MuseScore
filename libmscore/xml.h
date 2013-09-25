//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2004-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __XML_H__
#define __XML_H__

#include "mscore.h"
#include "spatium.h"
#include "fraction.h"
#include "property.h"

namespace Ms {

class Spanner;
class Beam;
class Tuplet;
class ClefList;

//---------------------------------------------------------
//   XmlReader
//---------------------------------------------------------

class XmlReader : public QXmlStreamReader {
      QString docName;  // used for error reporting

      // Score read context (for read optimizations):
      int _tick;
      int _track;
      QList<Beam*>    _beams;
      QList<Tuplet*>  _tuplets;

   public:
      XmlReader(QFile*);
      XmlReader(const QByteArray& d, const QString& s = QString());
      XmlReader(QIODevice* d, const QString& s = QString());
      XmlReader(const QString& d, const QString& s = QString());

      void unknown() const;

      // attribute helper routines:
      QString attribute(const char* s) const { return attributes().value(s).toString(); }
      QString attribute(const char* s, const QString&) const;
      int intAttribute(const char* s) const;
      int intAttribute(const char* s, int _default) const;
      double doubleAttribute(const char* s) const;
      double doubleAttribute(const char* s, double _default) const;
      bool hasAttribute(const char* s) const;

      // helper routines based on readElementText():
      int readInt()         { return readElementText().toInt();    }
      int readInt(bool* ok) { return readElementText().toInt(ok);  }
      double readDouble()   { return readElementText().toDouble(); }
      QPointF readPoint();
      QSizeF readSize();
      QRectF readRect();
      QColor readColor();
      Fraction readFraction();

      void setDocName(const QString& s) { docName = s; }

      int tick()  const           { return _tick;  }
      int& rtick()                { return _tick;  }
      void setTick(int val)       { _tick = val; }
      int track() const           { return _track; }
      void setTrack(int val)      { _track = val; }
      void addTuplet(Tuplet* s);
      void addBeam(Beam* s)       { _beams.append(s); }

      Beam* findBeam(int) const;
      Tuplet* findTuplet(int) const;

      QList<Tuplet*>& tuplets()        { return _tuplets; }
      QList<Beam*>& beams()            { return _beams; }
      };

//---------------------------------------------------------
//   Xml
//---------------------------------------------------------

class Xml : public QTextStream {
      static const int BS = 2048;

      QList<QString> stack;
      void putLevel();
      QList<Spanner*> _spanner;

   public:
      int curTick;            // used to optimize output
      int curTrack;
      int tickDiff;
      int trackDiff;          // saved track is curTrack-trackDiff

      bool clipboardmode;     // used to modify write() behaviour
      bool excerptmode;       // true when writing a part
      bool writeOmr;          // false if writing into *.msc file

      int tupletId;
      int beamId;
      int spannerId;
      QList<Spanner*>& spanner() { return _spanner; }
      void addSpanner(Spanner* s) { _spanner.append(s); }

      Xml(QIODevice* dev);
      Xml();

      void sTag(const char* name, Spatium sp) { Xml::tag(name, QVariant(sp.val())); }
      void pTag(const char* name, PlaceText);
      void fTag(const char* name, const Fraction&);

      void header();

      void stag(const QString&);
      void etag();

      void tagE(const QString&);
      void tagE(const char* format, ...);
      void ntag(const char* name);
      void netag(const char* name);

      void tag(P_ID id, void* data, void* defaultVal);
      void tag(P_ID id, QVariant data, QVariant defaultData = QVariant());
      void tag(const char* name, QVariant data, QVariant defaultData = QVariant());
      void tag(const QString&, QVariant data);
      void tag(const char* name, const char* s)    { tag(name, QVariant(s)); }
      void tag(const char* name, const QString& s) { tag(name, QVariant(s)); }
      void tag(const char* name, const QWidget*);

      void writeHtml(QString s);
      void dump(int len, const unsigned char* p);

      static QString xmlString(const QString&);
      static void htmlToString(XmlReader&, int level, QString*);
      static QString htmlToString(XmlReader&);
      };

extern PlaceText readPlacement(XmlReader&);
extern QString docName;

}     // namespace Ms
#endif

