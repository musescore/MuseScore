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
#include "io/iodevice.h"
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

namespace mu::engraving {
class XmlReader;
class XmlWriter;

class MStyle
{
public:
    MStyle() = default;

    const PropertyValue& styleV(Sid idx) const { return value(idx); }
    Spatium styleS(Sid idx) const
    {
        Q_ASSERT(MStyle::valueType(idx) == P_TYPE::SPATIUM);
        return value(idx).value<Spatium>();
    }

    Millimetre styleMM(Sid idx) const { Q_ASSERT(MStyle::valueType(idx) == P_TYPE::SPATIUM); return valueMM(idx); }
    String  styleSt(Sid idx) const { Q_ASSERT(MStyle::valueType(idx) == P_TYPE::STRING); return value(idx).value<String>(); }
    bool     styleB(Sid idx) const { Q_ASSERT(MStyle::valueType(idx) == P_TYPE::BOOL); return value(idx).toBool(); }
    qreal    styleD(Sid idx) const { Q_ASSERT(MStyle::valueType(idx) == P_TYPE::REAL); return value(idx).toReal(); }
    int      styleI(Sid idx) const { /* can be int or enum, so no assert */ return value(idx).toInt(); }

    const PropertyValue& value(Sid idx) const;
    Millimetre valueMM(Sid idx) const;

    void set(Sid idx, const PropertyValue& v);

    bool isDefault(Sid idx) const;
    void setDefaultStyleVersion(const int defaultsVersion);
    int defaultStyleVersion() const;

    bool read(mu::io::IODevice* device, bool ign = false);
    bool write(mu::io::IODevice* device);
    void save(XmlWriter& xml, bool optimize);
    static bool isValid(mu::io::IODevice* device);

    void precomputeValues();

    static P_TYPE valueType(const Sid);
    static const char* valueName(const Sid);
    static Sid styleIdx(const String& name);

private:

    friend class compat::ReadStyleHook;

    void read(XmlReader& e, compat::ReadChordListHook* readChordListHook);

    bool readProperties(XmlReader&);
    bool readStyleValCompat(XmlReader&);
    bool readTextStyleValCompat(XmlReader&);

    std::array<PropertyValue, size_t(Sid::STYLES)> m_values;
    std::array<Millimetre, size_t(Sid::STYLES)> m_precomputedValues;
};
} // namespace mu::engraving

#endif // MU_ENGRAVING_STYLE_H
