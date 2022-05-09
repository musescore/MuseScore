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
#ifndef MU_ENGRAVING_XMLWRITER_H
#define MU_ENGRAVING_XMLWRITER_H

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
class WriteContext;
}

namespace Ms {
class XmlWriter : public QTextStream
{
    static const int BS = 2048;

    Score* _score;
    std::list<QString> stack;
    SelectionFilter _filter;

    Fraction _curTick    { 0, 1 };       // used to optimize output
    Fraction _tickDiff   { 0, 1 };
    track_idx_t _curTrack = mu::nidx;
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

    track_idx_t curTrack() const { return _curTrack; }
    void setCurTrack(track_idx_t v) { _curTrack  = v; }

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
    void tag(const char* name, const CustDef& cd);

    void comment(const QString&);

    void writeXml(const QString&, QString s);
    void dump(int len, const unsigned char* p);

    void setFilter(SelectionFilter f) { _filter = f; }
    bool canWrite(const EngravingItem*) const;
    bool canWriteVoice(track_idx_t track) const;

    mu::engraving::WriteContext* context() const;
    void setContext(mu::engraving::WriteContext* context);

    static QString xmlString(const QString&);
    static QString xmlString(ushort c);
};
}

#endif // MU_ENGRAVING_XMLWRITER_H
