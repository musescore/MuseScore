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

#include <QMultiMap>

#include "connector.h"
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
class LinkedElements;

//---------------------------------------------------------
//   SpannerValues
//---------------------------------------------------------

struct SpannerValues {
      int spannerId;
      Fraction tick2;
      int track2;
      };

//---------------------------------------------------------
//   TextStyleMap
//---------------------------------------------------------

struct TextStyleMap {
      QString name;
      Tid ss;
      };

//---------------------------------------------------------
//   LinksIndexer
//---------------------------------------------------------

class LinksIndexer {
      int _lastLocalIndex              { -1                    };
      Location _lastLinkedElementLoc   { Location::absolute()  };

   public:
      int assignLocalIndex(const Location& mainElementInfo);
      };

//---------------------------------------------------------
//   XmlReader
//---------------------------------------------------------

class XmlReader : public QXmlStreamReader {
      QString docName;  // used for error reporting

      // Score read context (for read optimizations):
      Fraction _tick             { Fraction(0, 1) };
      Fraction _tickOffset       { Fraction(0, 1) };
      int _intTick          { 0       };
      int _track            { 0       };
      int _trackOffset      { 0       };
      bool _pasteMode       { false   };        // modifies read behaviour on paste operation
      Measure* _lastMeasure { 0       };
      Measure* _curMeasure  { 0       };
      int _curMeasureIdx    { 0       };
      QHash<int, Beam*>    _beams;
      QHash<int, Tuplet*>  _tuplets;

      QList<SpannerValues> _spannerValues;
      QList<std::pair<int,Spanner*>> _spanner;
      QList<StaffType> _staffTypes;
      QList<std::pair<Element*, QPointF>> _fixOffsets;

      std::vector<std::unique_ptr<ConnectorInfoReader>> _connectors;
      std::vector<std::unique_ptr<ConnectorInfoReader>> _pendingConnectors; // connectors that are pending to be updated and added to _connectors. That will happen when checkConnectors() is called.

      void htmlToString(int level, QString*);
      Interval _transpose;
      QMap<int, LinkedElements*> _elinks; // for reading old files (< 3.01)
      QMap<int, QList<QPair<LinkedElements*, Location>>> _staffLinkedElements; // one list per staff
      LinksIndexer _linksIndexer;
      QMultiMap<int, int> _tracks;

      QList<TextStyleMap> userTextStyles;

      void addConnectorInfo(std::unique_ptr<ConnectorInfoReader>);
      void removeConnector(const ConnectorInfoReader*); // Removes the whole ConnectorInfo chain from the connectors list.

      qint64 _offsetLines { 0 };

   public:
      XmlReader(QFile* f) : QXmlStreamReader(f), docName(f->fileName()) {}
      XmlReader(const QByteArray& d, const QString& st = QString()) : QXmlStreamReader(d), docName(st)  {}
      XmlReader(QIODevice* d, const QString& st = QString()) : QXmlStreamReader(d), docName(st) {}
      XmlReader(const QString& d, const QString& st = QString()) : QXmlStreamReader(d), docName(st) {}
      XmlReader(const XmlReader&) = delete;
      XmlReader& operator=(const XmlReader&) = delete;
      ~XmlReader();

      bool hasAccidental { false };                     // used for userAccidental backward compatibility
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
      int readInt()            { return readElementText().toInt();      }
      int readInt(bool* ok)    { return readElementText().toInt(ok);    }
      int readIntHex()         { return readElementText().toInt(0, 16); }
      double readDouble()      { return readElementText().toDouble();   }
      qlonglong readLongLong() { return readElementText().toLongLong(); }

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

      Fraction tick()  const            { return _tick + _tickOffset; }
      Fraction rtick()  const ;
      Fraction tickOffset() const       { return _tickOffset; }
      void setTick(const Fraction& f);
      void incTick(const Fraction& f);
      void setTickOffset(const Fraction& val) { _tickOffset = val; }

      int track() const            { return _track + _trackOffset;     }
      void setTrackOffset(int val) { _trackOffset = val;   }
      int trackOffset() const      { return _trackOffset;   }
      void setTrack(int val)       { _track = val;      }
      bool pasteMode() const       { return _pasteMode; }
      void setPasteMode(bool v)    { _pasteMode = v;    }

      Location location(bool forceAbsFrac = false) const;
      void fillLocation(Location&, bool forceAbsFrac = false) const;
      void setLocation(const Location&); // sets a new reading point, taking into
                                         // account its type (absolute or relative).

      void addBeam(Beam* s);
      Beam* findBeam(int id) const          { return _beams.value(id);   }

      void addTuplet(Tuplet* s);
      Tuplet* findTuplet(int id) const      { return _tuplets.value(id); }
      QHash<int, Tuplet*>& tuplets()        { return _tuplets; }

      void setLastMeasure(Measure* m)       { _lastMeasure = m;    }
      Measure* lastMeasure() const          { return _lastMeasure; }
      void setCurrentMeasure(Measure* m)    { _curMeasure = m; }
      Measure* currentMeasure() const       { return _curMeasure; }
      void setCurrentMeasureIndex(int idx)  { _curMeasureIdx = idx;  }
      int currentMeasureIndex() const       { return _curMeasureIdx; }

      void removeSpanner(const Spanner*);
      void addSpanner(int id, Spanner*);
      Spanner* findSpanner(int id);

      int spannerId(const Spanner*);      // returns spanner id, allocates new one if none exists

      void addSpannerValues(const SpannerValues& sv) { _spannerValues.append(sv); }
      const SpannerValues* spannerValues(int id) const;

      void addConnectorInfoLater(std::unique_ptr<ConnectorInfoReader> c) { _pendingConnectors.push_back(std::move(c)); } // add connector info to be checked after calling checkConnectors()
      void checkConnectors();
      void reconnectBrokenConnectors();

      QList<StaffType>& staffType()     { return _staffTypes; }
      Interval transpose() const        { return _transpose; }
      void setTransposeChromatic(int v) { _transpose.chromatic = v; }
      void setTransposeDiatonic(int v)  { _transpose.diatonic = v; }

      LinkedElements* getLink(bool masterScore, const Location& l, int localIndexDiff);
      void addLink(Staff* staff, LinkedElements* link);
      QMap<int, LinkedElements*>& linkIds() { return _elinks;     }
      QMultiMap<int, int>& tracks()         { return _tracks;     }

      void checkTuplets();
      Tid addUserTextStyle(const QString& name);
      Tid lookupUserTextStyle(const QString& name) const;
      void clearUserTextStyles() { userTextStyles.clear(); }

      QList<std::pair<Element*, QPointF>>& fixOffsets() { return  _fixOffsets; }

      // for reading old files (< 3.01)
      QMap<int, QList<QPair<LinkedElements*, Location>>>& staffLinkedElements() { return _staffLinkedElements; }
      void setOffsetLines(qint64 val) { _offsetLines = val; }
      };

//---------------------------------------------------------
//   XmlWriter
//---------------------------------------------------------

class XmlWriter : public QTextStream {
      static const int BS = 2048;

      Score* _score;
      QList<QString> stack;
      SelectionFilter _filter;

      Fraction _curTick    { 0, 1 };     // used to optimize output
      Fraction _tickDiff   { 0, 1 };
      int _curTrack        { -1 };
      int _trackDiff       { 0 };       // saved track is curTrack-trackDiff

      bool _clipboardmode  { false };   // used to modify write() behaviour
      bool _excerptmode    { false };   // true when writing a part
      bool _writeOmr       { true };    // false if writing into *.msc file
      bool _writeTrack     { false };
      bool _writePosition  { false };

      LinksIndexer _linksIndexer;
      QMap<int, int> _lidLocalIndices;

      std::vector<std::pair<const ScoreElement*, QString>> _elements;
      bool _recordElements = false;

      void putLevel();

   public:
      XmlWriter(Score*);
      XmlWriter(Score* s, QIODevice* dev);

      Fraction curTick() const            { return _curTick; }
      void setCurTick(const Fraction& v)  { _curTick   = v; }
      void incCurTick(const Fraction& v)  { _curTick += v; }

      int curTrack() const                { return _curTrack; }
      void setCurTrack(int v)             { _curTrack  = v; }

      Fraction tickDiff() const           { return _tickDiff; }
      void setTickDiff(const Fraction& v) { _tickDiff  = v; }

      int trackDiff() const               { return _trackDiff; }
      void setTrackDiff(int v)            { _trackDiff = v; }

      bool clipboardmode() const          { return _clipboardmode; }
      bool excerptmode() const            { return _excerptmode;   }
      bool writeOmr() const               { return _writeOmr;   }
      bool writeTrack() const             { return _writeTrack;    }
      bool writePosition() const          { return _writePosition; }

      void setClipboardmode(bool v)       { _clipboardmode = v; }
      void setExcerptmode(bool v)         { _excerptmode = v;   }
      void setWriteOmr(bool v)            { _writeOmr = v;      }
      void setWriteTrack(bool v)          { _writeTrack= v;     }
      void setWritePosition(bool v)       { _writePosition = v; }

      int assignLocalIndex(const Location& mainElementLocation);
      void setLidLocalIndex(int lid, int localIndex) { _lidLocalIndices.insert(lid, localIndex); }
      int lidLocalIndex(int lid) const { return _lidLocalIndices[lid]; }

      const std::vector<std::pair<const ScoreElement*, QString>>& elements() const { return _elements; }
      void setRecordElements(bool record) { _recordElements = record; }

      void sTag(const char* name, Spatium sp) { XmlWriter::tag(name, QVariant(sp.val())); }
      void pTag(const char* name, PlaceText);

      void header();

      void stag(const QString&);
      void etag();

      void stag(const ScoreElement* se, const QString& attributes = QString());
      void stag(const QString& name, const ScoreElement* se, const QString& attributes = QString());

      void tagE(const QString&);
      void tagE(const char* format, ...);
      void ntag(const char* name);
      void netag(const char* name);

      void tag(Pid id, void* data, void* defaultVal);
      void tag(Pid id, QVariant data, QVariant defaultData = QVariant());
      void tag(const char* name, QVariant data, QVariant defaultData = QVariant());
      void tag(const QString&, QVariant data);
      void tag(const char* name, const char* s)    { tag(name, QVariant(s)); }
      void tag(const char* name, const QString& s) { tag(name, QVariant(s)); }
      void tag(const char* name, const QWidget*);

      void comment(const QString&);

      void writeXml(const QString&, QString s);
      void dump(int len, const unsigned char* p);

      void setFilter(SelectionFilter f) { _filter = f; }
      bool canWrite(const Element*) const;
      bool canWriteVoice(int track) const;

      static QString xmlString(const QString&);
      static QString xmlString(ushort c);
      };

extern PlaceText readPlacement(XmlReader&);
}     // namespace Ms
#endif

