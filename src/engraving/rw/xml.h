/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __XML_H__
#define __XML_H__

#include <QMultiMap>
#include <QXmlStreamReader>
#include <QTextStream>
#include <QFile>

#include "infrastructure/draw/color.h"
#include "libmscore/connector.h"
#include "libmscore/stafftype.h"
#include "libmscore/interval.h"
#include "libmscore/engravingitem.h"
#include "libmscore/select.h"

namespace mu::engraving {
class ReadContext;
class WriteContext;
}

namespace Ms {
class Spanner;
class Beam;
class Tuplet;
class Measure;
class LinkedObjects;

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
    TextStyleType ss;
};

//---------------------------------------------------------
//   XmlReader
//---------------------------------------------------------

class XmlReader : public QXmlStreamReader
{
    QString docName;    // used for error reporting

    // Score read context (for read optimizations):
    Fraction _tick             { Fraction(0, 1) };
    Fraction _tickOffset       { Fraction(0, 1) };
    int _intTick          { 0 };
    int _track            { 0 };
    int _trackOffset      { 0 };
    bool _pasteMode       { false };            // modifies read behaviour on paste operation
    Measure* _lastMeasure { 0 };
    Measure* _curMeasure  { 0 };
    int _curMeasureIdx    { 0 };
    QHash<int, Beam*> _beams;
    QHash<int, Tuplet*> _tuplets;

    QList<SpannerValues> _spannerValues;
    QList<std::pair<int, Spanner*> > _spanner;
    QList<StaffType> _staffTypes;
    QList<std::pair<EngravingItem*, mu::PointF> > _fixOffsets;

    std::vector<std::unique_ptr<ConnectorInfoReader> > _connectors;
    std::vector<std::unique_ptr<ConnectorInfoReader> > _pendingConnectors;  // connectors that are pending to be updated and added to _connectors. That will happen when checkConnectors() is called.

    void htmlToString(int level, QString*);
    Interval _transpose;
    QMap<int, LinkedObjects*> _elinks;   // for reading old files (< 3.01)
    QMultiMap<int, int> _tracks;

    QList<TextStyleMap> userTextStyles;

    void addConnectorInfo(std::unique_ptr<ConnectorInfoReader>);
    void removeConnector(const ConnectorInfoReader*);   // Removes the whole ConnectorInfo chain from the connectors list.

    qint64 _offsetLines { 0 };

    mu::engraving::ReadContext* m_context = nullptr;

public:
    XmlReader(QFile* f)
        : QXmlStreamReader(f), docName(f->fileName()) {}
    XmlReader(const QByteArray& d)
        : QXmlStreamReader(d) {}
    XmlReader(QIODevice* d)
        : QXmlStreamReader(d) {}
    XmlReader(const QString& d)
        : QXmlStreamReader(d) {}

    XmlReader(const XmlReader&) = delete;
    XmlReader& operator=(const XmlReader&) = delete;

    ~XmlReader();

    bool hasAccidental { false };                       // used for userAccidental backward compatibility
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
    int readInt() { return readElementText().toInt(); }
    int readInt(bool* ok) { return readElementText().toInt(ok); }
    int readIntHex() { return readElementText().toInt(0, 16); }
    double readDouble() { return readElementText().toDouble(); }
    qlonglong readLongLong() { return readElementText().toLongLong(); }

    double readDouble(double min, double max);
    bool readBool();
    mu::PointF readPoint();
    mu::SizeF readSize();
    mu::ScaleF readScale();
    mu::RectF readRect();
    mu::draw::Color readColor();
    Fraction readFraction();
    QString readXml();

    void setDocName(const QString& s) { docName = s; }
    QString getDocName() const { return docName; }

    Fraction tick()  const { return _tick + _tickOffset; }
    Fraction rtick()  const;
    Fraction tickOffset() const { return _tickOffset; }
    void setTick(const Fraction& f);
    void incTick(const Fraction& f);
    void setTickOffset(const Fraction& val) { _tickOffset = val; }

    int track() const { return _track + _trackOffset; }
    void setTrackOffset(int val) { _trackOffset = val; }
    int trackOffset() const { return _trackOffset; }
    void setTrack(int val) { _track = val; }
    bool pasteMode() const { return _pasteMode; }
    void setPasteMode(bool v) { _pasteMode = v; }

    Location location(bool forceAbsFrac = false) const;
    void fillLocation(Location&, bool forceAbsFrac = false) const;
    void setLocation(const Location&);   // sets a new reading point, taking into
                                         // account its type (absolute or relative).

    void addBeam(Beam* s);
    Beam* findBeam(int id) const { return _beams.value(id); }

    void addTuplet(Tuplet* s);
    Tuplet* findTuplet(int id) const { return _tuplets.value(id); }
    QHash<int, Tuplet*>& tuplets() { return _tuplets; }

    void setLastMeasure(Measure* m) { _lastMeasure = m; }
    Measure* lastMeasure() const { return _lastMeasure; }
    void setCurrentMeasure(Measure* m) { _curMeasure = m; }
    Measure* currentMeasure() const { return _curMeasure; }
    void setCurrentMeasureIndex(int idx) { _curMeasureIdx = idx; }
    int currentMeasureIndex() const { return _curMeasureIdx; }

    void removeSpanner(const Spanner*);
    void addSpanner(int id, Spanner*);
    Spanner* findSpanner(int id);

    int spannerId(const Spanner*);        // returns spanner id, allocates new one if none exists

    void addSpannerValues(const SpannerValues& sv) { _spannerValues.append(sv); }
    const SpannerValues* spannerValues(int id) const;

    void addConnectorInfoLater(std::unique_ptr<ConnectorInfoReader> c) { _pendingConnectors.push_back(std::move(c)); }   // add connector info to be checked after calling checkConnectors()
    void checkConnectors();
    void reconnectBrokenConnectors();

    QList<StaffType>& staffType() { return _staffTypes; }
    Interval transpose() const { return _transpose; }
    void setTransposeChromatic(int v) { _transpose.chromatic = v; }
    void setTransposeDiatonic(int v) { _transpose.diatonic = v; }

    QMap<int, LinkedObjects*>& linkIds() { return _elinks; }
    QMultiMap<int, int>& tracks() { return _tracks; }

    void checkTuplets();
    TextStyleType addUserTextStyle(const QString& name);
    TextStyleType lookupUserTextStyle(const QString& name) const;
    void clearUserTextStyles() { userTextStyles.clear(); }

    QList<std::pair<EngravingItem*, mu::PointF> >& fixOffsets() { return _fixOffsets; }

    // for reading old files (< 3.01)
    void setOffsetLines(qint64 val) { _offsetLines = val; }

    mu::engraving::ReadContext* context() const;
    void setContext(mu::engraving::ReadContext* context);
};

//---------------------------------------------------------
//   XmlWriter
//---------------------------------------------------------

class XmlWriter : public QTextStream
{
    static const int BS = 2048;

    Score* _score;
    QList<QString> stack;
    SelectionFilter _filter;

    Fraction _curTick    { 0, 1 };       // used to optimize output
    Fraction _tickDiff   { 0, 1 };
    int _curTrack        { -1 };
    int _trackDiff       { 0 };         // saved track is curTrack-trackDiff

    bool _clipboardmode  { false };     // used to modify write() behaviour
    bool _excerptmode    { false };     // true when writing a part
    bool _msczMode       { true };      // false if writing into *.msc file
    bool _writeTrack     { false };
    bool _writePosition  { false };

    std::vector<std::pair<const EngravingObject*, QString> > _elements;
    bool _recordElements = false;

    mu::engraving::WriteContext* m_context = nullptr;

    void putLevel();

public:
    XmlWriter(Score*);
    XmlWriter(Score* s, QIODevice* dev);

    Fraction curTick() const { return _curTick; }
    void setCurTick(const Fraction& v) { _curTick   = v; }
    void incCurTick(const Fraction& v) { _curTick += v; }

    int curTrack() const { return _curTrack; }
    void setCurTrack(int v) { _curTrack  = v; }

    Fraction tickDiff() const { return _tickDiff; }
    void setTickDiff(const Fraction& v) { _tickDiff  = v; }

    int trackDiff() const { return _trackDiff; }
    void setTrackDiff(int v) { _trackDiff = v; }

    bool clipboardmode() const { return _clipboardmode; }
    bool excerptmode() const { return _excerptmode; }
    bool isMsczMode() const { return _msczMode; }
    bool writeTrack() const { return _writeTrack; }
    bool writePosition() const { return _writePosition; }

    void setClipboardmode(bool v) { _clipboardmode = v; }
    void setExcerptmode(bool v) { _excerptmode = v; }
    void setIsMsczMode(bool v) { _msczMode = v; }
    void setWriteTrack(bool v) { _writeTrack= v; }
    void setWritePosition(bool v) { _writePosition = v; }

    const std::vector<std::pair<const EngravingObject*, QString> >& elements() const { return _elements; }
    void setRecordElements(bool record) { _recordElements = record; }

    void writeHeader();

    void startObject(const QString&);
    void endObject();

    void startObject(const EngravingObject* se, const QString& attributes = QString());
    void startObject(const QString& name, const EngravingObject* se, const QString& attributes = QString());

    void tagE(const QString&);
    void tagE(const char* format, ...);
    void ntag(const char* name);
    void netag(const char* name);

    void tag(Pid id, const mu::engraving::PropertyValue& data, const mu::engraving::PropertyValue& def = mu::engraving::PropertyValue());
    void tagProperty(const char* name, mu::engraving::P_TYPE type, const mu::engraving::PropertyValue& data);

    void tag(const char* name, QVariant data, QVariant defaultData = QVariant());
    void tag(const QString&, QVariant data);
    void tag(const char* name, const char* s) { tag(name, QVariant(s)); }
    void tag(const char* name, const QString& s) { tag(name, QVariant(s)); }
    void tag(const char* name, const mu::PointF& v);
    void tag(const char* name, const Fraction& v, const Fraction& def = Fraction());

    void comment(const QString&);

    void writeXml(const QString&, QString s);
    void dump(int len, const unsigned char* p);

    void setFilter(SelectionFilter f) { _filter = f; }
    bool canWrite(const EngravingItem*) const;
    bool canWriteVoice(int track) const;

    mu::engraving::WriteContext* context() const;
    void setContext(mu::engraving::WriteContext* context);

    static QString xmlString(const QString&);
    static QString xmlString(ushort c);
};
}     // namespace Ms
#endif
