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

#include "thirdparty/xmlstream/xmlstream.h"
#include "stafftype.h"
#include "interval.h"
#include "element.h"
#include "select.h"

namespace Ms {

enum class PlaceText : char;
enum class ClefType : signed char;
class Spanner;
class Beam;
class Tuplet;
class Measure;

//---------------------------------------------------------
//   SpannerValues
//---------------------------------------------------------

struct SpannerValues {
      int spannerId;
      int tick2;
      int track2;
      };

//---------------------------------------------------------
//   XmlReader
//---------------------------------------------------------

class XmlReader : public XmlStreamReader {
      QString docName;  // used for error reporting

      // Score read context (for read optimizations):
      int _tick             { 0       };
      int _tickOffset       { 0       };
      int _track            { 0       };
      int _trackOffset      { 0       };
      bool _pasteMode       { false   };        // modifies read behaviour on paste operation
      Measure* _lastMeasure { nullptr };
      QHash<int, Beam*>    _beams;
      QHash<int, Tuplet*>  _tuplets;
      QList<SpannerValues> _spannerValues;
      QList<std::pair<int,Spanner*>> _spanner;
      QList<StaffType> _staffTypes;
      void htmlToString(int level, QString*);
      Interval _transpose;
      QList<QList<std::pair<int, ClefType>>> _clefs;   // for 1.3 scores

   public:
      XmlReader(QFile* f) : XmlStreamReader(f), docName(f->fileName()) {}
      XmlReader(const QByteArray& d, const QString& s = QString()) : XmlStreamReader(d), docName(s)  {}
      XmlReader(QIODevice* d, const QString& s = QString()) : XmlStreamReader(d), docName(s) {}
      XmlReader(const QString& d, const QString& s = QString()) : XmlStreamReader(d), docName(s) {}

      void unknown();

      // attribute helper routines:
      QString attribute(const char* s) const { return attributes().value(s).toString(); }
      QString attribute(const char* s, const QString&) const;
      int intAttribute(const char* s) const;
      int intAttribute(const char* s, int _default) const;
      double doubleAttribute(const char* s) const;
      double doubleAttribute(const char* s, double _default) const;
      bool hasAttribute(const char* s) const;

      // helper routines based on readElementText():
      int readInt()         { return readElementText().toInt();      }
      int readInt(bool* ok) { return readElementText().toInt(ok);    }
      int readIntHex()      { return readElementText().toInt(0, 16); }
      double readDouble()   { return readElementText().toDouble();   }
      double readDouble(double min, double max);
      bool readBool();
      QPointF readPoint();
      QSizeF readSize();
      QRectF readRect();
      QColor readColor();
      Fraction readFraction();
      QString readXml();

      void setDocName(const QString& s) { docName = s; }
      QString getDocName() const        { return docName; }

      int tick()  const            { return _tick + _tickOffset;  }
      void initTick(int val)       { _tick = val;       }
      void incTick(int val)        { _tick += val;      }
      void setTickOffset(int val)  { _tickOffset = val; }
      int track() const            { return _track + _trackOffset;     }
      void setTrackOffset(int val) { _trackOffset = val;   }
      int trackOffset() const      { return _trackOffset;   }
      void setTrack(int val)       { _track = val;      }
      bool pasteMode() const       { return _pasteMode; }
      void setPasteMode(bool v)    { _pasteMode = v;    }

      void addBeam(Beam* s);
      Beam* findBeam(int id) const { return _beams.value(id);   }

      void addTuplet(Tuplet* s);
      Tuplet* findTuplet(int id) const { return _tuplets.value(id); }
      QHash<int, Tuplet*>& tuplets()   { return _tuplets; }

      void setLastMeasure(Measure* m) { _lastMeasure = m;    }
      Measure* lastMeasure() const    { return _lastMeasure; }

      void removeSpanner(const Spanner*);
      void addSpanner(int id, Spanner*);
      Spanner* findSpanner(int id);
      int spannerId(const Spanner*);      // returns spanner id, allocates new one if none exists

      void addSpannerValues(const SpannerValues& sv) { _spannerValues.append(sv); }
      const SpannerValues* spannerValues(int id) const;
      QList<StaffType>& staffType() { return _staffTypes; }
      Interval transpose() const { return _transpose; }
      void setTransposeChromatic(int v) { _transpose.chromatic = v; }
      void setTransposeDiatonic(int v) { _transpose.diatonic = v; }

      QList<std::pair<int, ClefType>>& clefs(int idx);
      void checkTuplets();
      };

//---------------------------------------------------------
//   Xml
//    xml writer
//---------------------------------------------------------

class Xml : public QTextStream {
      static const int BS = 2048;

      QList<QString> stack;
      QList<std::pair<int,const Spanner*>> _spanner;
      int _spannerId = 1;
      SelectionFilter _filter;

   public:
      int curTick   =  0;           // used to optimize output
      int curTrack  = -1;
      int tickDiff  =  0;
      int trackDiff =  0;           // saved track is curTrack-trackDiff

      bool clipboardmode = false;   // used to modify write() behaviour
      bool excerptmode   = false;   // true when writing a part
      bool writeOmr      = true;    // false if writing into *.msc file

      int tupletId  = 1;
      int beamId    = 1;

      int addSpanner(const Spanner*);     // returns allocated id
      const Spanner* findSpanner(int id);
      int spannerId(const Spanner*);      // returns spanner id, allocates new one if none exists

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

      void writeXml(const QString&, QString s);
      void dump(int len, const unsigned char* p);

      void setFilter(SelectionFilter f) { _filter = f; }
      bool canWrite(const Element*) const;
      bool canWriteVoice(int track) const;

      static QString xmlString(const QString&);
      static QString xmlString(ushort c);

      void putLevel();
      };

extern PlaceText readPlacement(XmlReader&);
extern QString docName;

}     // namespace Ms
#endif

