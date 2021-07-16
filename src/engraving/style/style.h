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

#include <functional>

#include <array>
#include <QFile>
#include <QSet>

#include "libmscore/types.h"
#include "libmscore/spatium.h"

#include "draw/geometry.h"

#include "styledef.h"

namespace mu::engraving::compat {
struct ReadChordListHook;
struct WriteChordListHook;
}

namespace Ms {
class XmlReader;
class XmlWriter;

class MStyle
{
public:
    MStyle();

    //! TODO Can be optimized
    const QVariant& styleV(Sid idx) const { return value(idx); }
    Spatium  styleS(Sid idx) const { Q_ASSERT(!strcmp(MStyle::valueType(idx), "Ms::Spatium")); return value(idx).value<Spatium>(); }
    qreal    styleP(Sid idx) const { Q_ASSERT(!strcmp(MStyle::valueType(idx), "Ms::Spatium")); return pvalue(idx); }
    QString  styleSt(Sid idx) const { Q_ASSERT(!strcmp(MStyle::valueType(idx), "QString")); return value(idx).toString(); }
    bool     styleB(Sid idx) const { Q_ASSERT(!strcmp(MStyle::valueType(idx), "bool")); return value(idx).toBool(); }
    qreal    styleD(Sid idx) const { Q_ASSERT(!strcmp(MStyle::valueType(idx), "double")); return value(idx).toDouble(); }
    int      styleI(Sid idx) const { Q_ASSERT(!strcmp(MStyle::valueType(idx), "int")); return value(idx).toInt(); }

    const QVariant& value(Sid idx) const;
    qreal pvalue(Sid idx) const;

    void set(Sid idx, const QVariant& v);
    void set(Sid idx, const mu::PointF& v);

    bool isDefault(Sid idx) const;
    void setDefaultStyleVersion(const int defaultsVersion);
    int defaultStyleVersion() const;

    bool load(QFile* qf, bool ign = false);
    void load(XmlReader& e, mu::engraving::compat::ReadChordListHook* readChordListHook);
    void save(XmlWriter& xml, bool optimize, mu::engraving::compat::WriteChordListHook* writeChordListHook);
    bool readProperties(XmlReader&);

    void precomputeValues();

    static const char* valueType(const Sid);
    static const char* valueName(const Sid);
    static Sid styleIdx(const QString& name);

private:
    bool readStyleValCompat(XmlReader&);
    bool readTextStyleValCompat(XmlReader&);

    std::array<QVariant, int(Sid::STYLES)> m_values;
    std::array<qreal, int(Sid::STYLES)> m_precomputedValues;

    int m_defaultStyleVersion = -1;
};
}     // namespace Ms

#endif // MU_ENGRAVING_STYLE_H
