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

#ifndef MU_ENGRAVING_STYLE_H
#define MU_ENGRAVING_STYLE_H

#include <array>
#include <QFile>
#include <QSet>

#include "libmscore/chordlist.h"
#include "libmscore/types.h"

#include "draw/geometry.h"

#include "styledef.h"

namespace Ms {
class XmlWriter;
struct ChordDescription;
class Element;
class Score;

//---------------------------------------------------------
//   MStyle
///   \cond PLUGIN_API \private \endcond
//    the name "Style" gives problems with some microsoft
//    header files...
//---------------------------------------------------------

class MStyle
{
    std::array<QVariant, int(Sid::STYLES)> _values;
    std::array<qreal, int(Sid::STYLES)> _precomputedValues;

    ChordList _chordList;
    bool _customChordList;          // if true, chordlist will be saved as part of score
    int _defaultStyleVersion = -1;

public:
    MStyle();

    void precomputeValues();
    const QVariant& value(Sid idx) const;
    qreal pvalue(Sid idx) const { return _precomputedValues[int(idx)]; }
    void set(Sid idx, const QVariant& v);
    void set(Sid idx, const mu::PointF& v);

    bool isDefault(Sid idx) const;
    void setDefaultStyleVersion(const int defaultsVersion);
    int defaultStyleVersion() const { return _defaultStyleVersion; }

    const ChordDescription* chordDescription(int id) const;
    ChordList* chordList() { return &_chordList; }

    void setCustomChordList(bool t) { _customChordList = t; }
    void checkChordList();

    bool load(QFile* qf, bool ign = false);
    void load(XmlReader& e);
    void applyNewDefaults(const MStyle& other, const int defaultsVersion);
    void save(XmlWriter& xml, bool optimize);
    bool readProperties(XmlReader&);
    bool readStyleValCompat(XmlReader&);
    bool readTextStyleValCompat(XmlReader&);

    void resetAllStyles(Score* score, const QSet<Sid>& ignoredStyles = QSet<Sid>());
    void resetStyles(Score* score, const QSet<Sid>& stylesToReset);

    static const char* valueType(const Sid);
    static const char* valueName(const Sid);
    static Sid styleIdx(const QString& name);
    static MStyle* resolveStyleDefaults(const int defaultsVersion);
};
}     // namespace Ms

#endif // MU_ENGRAVING_STYLE_H
