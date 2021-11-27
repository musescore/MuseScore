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
#include <QIODevice>
#include <QSet>

#include "libmscore/types.h"
#include "types/dimension.h"

#include "property/propertyvalue.h"
#include "infrastructure/draw/geometry.h"

#include "styledef.h"

namespace mu::engraving::compat {
class ReadChordListHook;
class ReadStyleHook;
}

namespace Ms {
class XmlReader;
class XmlWriter;

class MStyle
{
public:
    MStyle() = default;

    const mu::engraving::PropertyValue& styleV(Sid idx) const { return value(idx); }
    Spatium styleS(Sid idx) const
    {
        Q_ASSERT(MStyle::valueType(idx) == mu::engraving::P_TYPE::SPATIUM);
        return value(idx).value<Spatium>();
    }

    Millimetre styleMM(Sid idx) const { Q_ASSERT(MStyle::valueType(idx) == mu::engraving::P_TYPE::SPATIUM); return valueMM(idx); }
    QString  styleSt(Sid idx) const { Q_ASSERT(MStyle::valueType(idx) == mu::engraving::P_TYPE::STRING); return value(idx).toString(); }
    bool     styleB(Sid idx) const { Q_ASSERT(MStyle::valueType(idx) == mu::engraving::P_TYPE::BOOL); return value(idx).toBool(); }
    qreal    styleD(Sid idx) const { Q_ASSERT(MStyle::valueType(idx) == mu::engraving::P_TYPE::REAL); return value(idx).toReal(); }
    int      styleI(Sid idx) const { /* can be int or enum, so no assert */ return value(idx).toInt(); }

    const mu::engraving::PropertyValue& value(Sid idx) const;
    Millimetre valueMM(Sid idx) const;

    void set(Sid idx, const mu::engraving::PropertyValue& v);

    bool isDefault(Sid idx) const;
    void setDefaultStyleVersion(const int defaultsVersion);
    int defaultStyleVersion() const;

    bool read(QIODevice* device, bool ign = false);
    bool write(QIODevice* device);
    void save(XmlWriter& xml, bool optimize);
    static bool isValid(QIODevice* device);

    void precomputeValues();

    static mu::engraving::P_TYPE valueType(const Sid);
    static const char* valueName(const Sid);
    static Sid styleIdx(const QString& name);

private:

    friend class mu::engraving::compat::ReadStyleHook;

    void read(XmlReader& e, mu::engraving::compat::ReadChordListHook* readChordListHook);

    bool readProperties(XmlReader&);
    bool readStyleValCompat(XmlReader&);
    bool readTextStyleValCompat(XmlReader&);

    std::array<mu::engraving::PropertyValue, size_t(Sid::STYLES)> m_values;
    std::array<Millimetre, size_t(Sid::STYLES)> m_precomputedValues;
};
}     // namespace Ms

#endif // MU_ENGRAVING_STYLE_H
