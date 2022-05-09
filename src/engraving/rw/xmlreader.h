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
#ifndef MU_ENGRAVING_XMLREADER_H
#define MU_ENGRAVING_XMLREADER_H

#include <map>
#include <QXmlStreamReader>
#include <QTextStream>
#include <QFile>

#include <unordered_map>

#include "containers.h"

#include "infrastructure/draw/color.h"
#include "libmscore/connector.h"
#include "libmscore/stafftype.h"
#include "libmscore/interval.h"
#include "libmscore/engravingitem.h"
#include "libmscore/select.h"

namespace mu::engraving {
class ReadContext;
}

namespace Ms {
struct SpannerValues {
    int spannerId;
    Fraction tick2;
    track_idx_t track2;
};

struct TextStyleMap {
    QString name;
    TextStyleType ss;
};

class XmlReader : public QXmlStreamReader
{
    QString docName;    // used for error reporting

    // Score read context (for read optimizations):
    Fraction _tick             { Fraction(0, 1) };
    Fraction _tickOffset       { Fraction(0, 1) };
    int _intTick          { 0 };
    track_idx_t _track = 0;
    int _trackOffset      { 0 };
    bool _pasteMode       { false };            // modifies read behaviour on paste operation
    Measure* _lastMeasure { 0 };
    Measure* _curMeasure  { 0 };
    int _curMeasureIdx    { 0 };
    std::unordered_map<int, Beam*> _beams;
    std::unordered_map<int, Tuplet*> _tuplets;

    std::list<SpannerValues> _spannerValues;
    std::list<std::pair<int, Spanner*> > _spanner;
    std::list<std::pair<EngravingItem*, mu::PointF> > _fixOffsets;

    std::vector<std::unique_ptr<ConnectorInfoReader> > _connectors;
    std::vector<std::unique_ptr<ConnectorInfoReader> > _pendingConnectors;  // connectors that are pending to be updated and added to _connectors. That will happen when checkConnectors() is called.

    void htmlToString(int level, QString*);
    Interval _transpose;
    std::map<int, LinkedObjects*> _elinks;   // for reading old files (< 3.01)
    TracksMap _tracks;

    std::list<TextStyleMap> userTextStyles;

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

    track_idx_t track() const { return _track + _trackOffset; }
    void setTrackOffset(int val) { _trackOffset = val; }
    int trackOffset() const { return _trackOffset; }
    void setTrack(track_idx_t val) { _track = val; }
    bool pasteMode() const { return _pasteMode; }
    void setPasteMode(bool v) { _pasteMode = v; }

    Location location(bool forceAbsFrac = false) const;
    void fillLocation(Location&, bool forceAbsFrac = false) const;
    void setLocation(const Location&);   // sets a new reading point, taking into
                                         // account its type (absolute or relative).

    void addBeam(Beam* s);
    Beam* findBeam(int id) const { return mu::value(_beams, id, nullptr); }

    void addTuplet(Tuplet* s);
    Tuplet* findTuplet(int id) const { return mu::value(_tuplets, id, nullptr); }
    std::unordered_map<int, Tuplet*>& tuplets() { return _tuplets; }

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

    void addSpannerValues(const SpannerValues& sv) { _spannerValues.push_back(sv); }
    const SpannerValues* spannerValues(int id) const;

    void addConnectorInfoLater(std::unique_ptr<ConnectorInfoReader> c) { _pendingConnectors.push_back(std::move(c)); }   // add connector info to be checked after calling checkConnectors()
    void checkConnectors();
    void reconnectBrokenConnectors();

    Interval transpose() const { return _transpose; }
    void setTransposeChromatic(int8_t v) { _transpose.chromatic = v; }
    void setTransposeDiatonic(int8_t v) { _transpose.diatonic = v; }

    std::map<int, LinkedObjects*>& linkIds() { return _elinks; }
    TracksMap& tracks() { return _tracks; }

    void checkTuplets();
    TextStyleType addUserTextStyle(const QString& name);
    TextStyleType lookupUserTextStyle(const QString& name) const;
    void clearUserTextStyles() { userTextStyles.clear(); }

    std::list<std::pair<EngravingItem*, mu::PointF> >& fixOffsets() { return _fixOffsets; }

    // for reading old files (< 3.01)
    void setOffsetLines(qint64 val) { _offsetLines = val; }

    mu::engraving::ReadContext* context() const;
    void setContext(mu::engraving::ReadContext* context);
};
}

#endif // MU_ENGRAVING_XMLREADER_H
