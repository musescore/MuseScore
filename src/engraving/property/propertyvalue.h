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
#ifndef MU_ENGRAVING_PROPERTYVALUE_H
#define MU_ENGRAVING_PROPERTYVALUE_H

#include <variant>
#include <any>
#include <string>
#include <memory>

#include <QVariant>

//#include "libmscore/property.h"
#include "libmscore/spatium.h"
#include "libmscore/types.h"
#include "libmscore/symid.h"
#include "libmscore/pitchvalue.h"
#include "libmscore/fraction.h"
//#include "libmscore/groups.h" can't be included
#include "infrastructure/draw/geometry.h"
#include "infrastructure/draw/painterpath.h"
#include "infrastructure/draw/color.h"

#include "framework/global/logstream.h"

namespace Ms {
class Groups;
class TDuration;

enum class P_TYPE {
    UNDEFINED = 0,
    // base
    BOOL,
    INT,
    REAL,
    STRING,
    // geometry
    POINT,
    SIZE,
    PATH,               // mu::PainterPath
    SCALE,
    SPATIUM,

    SIZE_MM,
    POINT_SP,           // point units, value saved in (score) spatium units
    POINT_MM,
    POINT_SP_MM,        // point units, value saved as mm or spatium depending on EngravingItem->sizeIsSpatiumDependent()
    SP_REAL,            // real (point) value saved in (score) spatium units

    // draw
    COLOR,
    FONT,
    ALIGN,
    PLACEMENT,        // ABOVE or BELOW
    HPLACEMENT,       // LEFT, CENTER or RIGHT
    DIRECTION,        // enum class Direction
    DIRECTION_H,      // enum class MScore::DirectionH

    // time
    FRACTION,

    ORNAMENT_STYLE,   // enum class MScore::OrnamentStyle
    TDURATION,
    LAYOUT_BREAK,
    VALUE_TYPE,
    BEAM_MODE,

    TEXT_PLACE,
    TEMPO,
    GROUPS,
    SYMID,
    INT_LIST,
    GLISS_STYLE,
    BARLINE_TYPE,
    HEAD_TYPE,          // enum class Notehead::Type
    HEAD_GROUP,         // enum class Notehead::Group
    ZERO_INT,           // displayed with offset +1

    SUB_STYLE,

    CHANGE_METHOD,      // enum class VeloChangeMethod (for single note dynamics)
    CHANGE_SPEED,       // enum class Dynamic::Speed
    CLEF_TYPE,          // enum class ClefType
    DYNAMIC_TYPE,       // enum class DynamicType
    KEYMODE,            // enum class KeyMode
    ORIENTATION,        // enum class Orientation

    HEAD_SCHEME,        // enum class NoteHead::Scheme

    PITCH_VALUES,
    HOOK_TYPE
};
}

namespace mu::engraving {
class PropertyValue
{
public:
    PropertyValue();
    // base
    PropertyValue(bool v);
    PropertyValue(int v);
    PropertyValue(qreal v);
    PropertyValue(const char* v);
    PropertyValue(const QString& v);
    // geometry
    PropertyValue(const PointF& v);
    PropertyValue(const SizeF& v);
    PropertyValue(const PainterPath& v);

    PropertyValue(const draw::Color& v);
    PropertyValue(Ms::Align v);
    PropertyValue(Ms::Direction v);
    PropertyValue(Ms::SymId v);
    PropertyValue(Ms::BarLineType v);
    PropertyValue(Ms::HookType v);
    PropertyValue(Ms::HPlacement v);
    PropertyValue(Ms::DynamicType v)
        : m_type(Ms::P_TYPE::DYNAMIC_TYPE), m_val(v) {}
    PropertyValue(const Ms::Spatium& v);
    PropertyValue(const Ms::PitchValues& v);
    PropertyValue(const Ms::Fraction& v);
    PropertyValue(const QList<int>& v); //endings

    PropertyValue(const Ms::Groups& v);
    PropertyValue(const Ms::TDuration& v);

    bool isValid() const;

    Ms::P_TYPE type() const;

    template<typename T>
    T value() const
    {
        if (Ms::P_TYPE::UNDEFINED == m_type) {
            return T();
        }

//        const T* pv = std::get_if<T>(&m_val);
//        assert(pv);
//        return pv ? *pv : T();

        try {
            return std::any_cast<T>(m_val);
        }
        catch (const std::bad_any_cast&) {
            return T();
        }
    }

    bool toBool() const { return value<bool>(); }
    int toInt() const { return value<int>(); }
    qreal toReal() const { return value<qreal>(); }
    double toDouble() const { return value<double>(); }
    QString toString() const { return value<QString>(); }
    Ms::Spatium toSpatium() const { return value<Ms::Spatium>(); }
    Ms::Align toAlign() const { return value<Ms::Align>(); }
    Ms::Direction toDirection() const { return value<Ms::Direction>(); }

    const Ms::Groups& toGroups() const;
    const Ms::TDuration& toTDuration() const;

    inline bool operator ==(const PropertyValue& v) const { return m_type == v.m_type /* && m_val == v.m_val*/; }
    inline bool operator !=(const PropertyValue& v) const { return !this->operator ==(v); }

    template<typename T>
    static PropertyValue fromValue(const T& v) { return PropertyValue(v); }

    //! NOTE compat
    QVariant toQVariant() const;
    static PropertyValue fromQVariant(const QVariant& v);

private:
    Ms::P_TYPE m_type = Ms::P_TYPE::UNDEFINED;
//    std::variant<bool, int, qreal, QString,
//                 Ms::Spatium,
//                 PointF, SizeF, PainterPath, draw::Color,
//                 Ms::Align, Ms::Direction, Ms::SymId, Ms::BarLineType, Ms::HookType, Ms::HPlacement,
//                 Ms::DynamicType,
//                 Ms::PitchValues, Ms::Fraction, QList<int>
//                 > m_val;

    std::any m_val;

    //! NOTE Temporary solution for some types
    std::any m_any;
};
}

inline mu::logger::Stream& operator<<(mu::logger::Stream& s, const mu::engraving::PropertyValue&)
{
    s << "property(not implemented log output)";
    return s;
}

#endif // MU_ENGRAVING_PROPERTYVALUE_H
