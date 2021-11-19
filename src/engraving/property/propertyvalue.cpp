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
#include "propertyvalue.h"

#include "realfn.h"

#include "libmscore/groups.h"

#include "log.h"

using namespace mu::engraving;

PropertyValue::PropertyValue()
{
}

PropertyValue::PropertyValue(bool v)
    : m_type(P_TYPE::BOOL), m_val(v)
{
}

PropertyValue::PropertyValue(int v)
    : m_type(P_TYPE::INT), m_val(v)
{
}

PropertyValue::PropertyValue(qreal v)
    : m_type(P_TYPE::REAL), m_val(v)
{
}

PropertyValue::PropertyValue(const char* v)
    : m_type(P_TYPE::STRING), m_val(QString(v))
{
}

PropertyValue::PropertyValue(const QString& v)
    : m_type(P_TYPE::STRING), m_val(v)
{
}

PropertyValue::PropertyValue(const Spatium& v)
    : m_type(P_TYPE::SPATIUM), m_val(v)
{
}

PropertyValue::PropertyValue(const PointF& v)
    : m_type(P_TYPE::POINT), m_val(v)
{
}

PropertyValue::PropertyValue(const SizeF& v)
    : m_type(P_TYPE::SIZE), m_val(v)
{
}

PropertyValue::PropertyValue(const PainterPath& v)
    : m_type(P_TYPE::PATH), m_val(v)
{
}

PropertyValue::PropertyValue(const draw::Color& v)
    : m_type(P_TYPE::COLOR), m_val(v)
{
}

PropertyValue::PropertyValue(Align v)
    : m_type(P_TYPE::ALIGN), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::Direction v)
    : m_type(P_TYPE::DIRECTION), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::SymId v)
    : m_type(P_TYPE::SYMID), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::BarLineType v)
    : m_type(P_TYPE::BARLINE_TYPE), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::HookType v)
    : m_type(P_TYPE::HOOK_TYPE), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::HPlacement v)
    : m_type(P_TYPE::HPLACEMENT), m_val(v)
{
}

PropertyValue::PropertyValue(const Ms::PitchValues& v)
    : m_type(P_TYPE::PITCH_VALUES), m_val(v)
{
}

PropertyValue::PropertyValue(const Ms::Groups& v)
    : m_type(P_TYPE::GROUPS), m_any(v)
{
}

const Ms::Groups& PropertyValue::toGroups() const
{
    return std::any_cast<const Ms::Groups&>(m_any);
}

PropertyValue::PropertyValue(const Ms::TDuration& v)
    : m_type(P_TYPE::TDURATION), m_any(v)
{
}

const Ms::TDuration& PropertyValue::toTDuration() const
{
    return std::any_cast<const Ms::TDuration&>(m_any);
}

PropertyValue::PropertyValue(const Ms::Fraction& v)
    : m_type(P_TYPE::FRACTION), m_val(v)
{
}

PropertyValue::PropertyValue(const QList<int>& v)
    : m_type(P_TYPE::INT_LIST), m_val(v)
{
}

bool PropertyValue::isValid() const
{
    return m_type != P_TYPE::UNDEFINED;
}

P_TYPE PropertyValue::type() const
{
    return m_type;
}

bool PropertyValue::operator ==(const PropertyValue& v) const
{
    //! HACK Temporary hack for bool comparisons (maybe one type is bool and another type is int)
    if (v.m_type == P_TYPE::BOOL || m_type == P_TYPE::BOOL) {
        return v.value<bool>() == value<bool>();
    }

    //! HACK Temporary hack for Spatium comparisons (maybe one type is Spatium and another type is real)
    if (v.m_type == P_TYPE::SPATIUM || m_type == P_TYPE::SPATIUM) {
        return RealIsEqual(v.value<qreal>(), value<qreal>());
    }

    //! HACK Temporary hack for Fraction comparisons
    if (v.m_type == P_TYPE::FRACTION) {
        assert(m_type == P_TYPE::FRACTION);
        return v.value<Ms::Fraction>().identical(value<Ms::Fraction>());
    }

    //! HACK Temporary hack for TDURATION comparisons
    if (v.m_type == P_TYPE::TDURATION) {
        assert(m_type == P_TYPE::TDURATION);
        return v.toTDuration() == toTDuration();
    }

    //! HACK Temporary hack for GROUPS comparisons
    if (v.m_type == P_TYPE::GROUPS) {
        assert(m_type == P_TYPE::GROUPS);
        return v.toGroups() == toGroups();
    }

    if (v.m_type == P_TYPE::REAL) {
        assert(m_type == P_TYPE::REAL);
        return RealIsEqual(v.value<qreal>(), value<qreal>());
    }

    return v.m_type == m_type && v.m_val == m_val;
}

QVariant PropertyValue::toQVariant() const
{
    switch (m_type) {
    case P_TYPE::UNDEFINED: return QVariant();
    // base
    case P_TYPE::BOOL:      return value<bool>();
    case P_TYPE::INT:       return value<int>();
    case P_TYPE::REAL:      return value<qreal>();
    case P_TYPE::STRING:    return value<QString>();
    // geometry
    case P_TYPE::POINT:     return value<PointF>().toQPointF();
    case P_TYPE::SIZE:      return value<SizeF>().toQSizeF();
    case P_TYPE::PATH: {
        UNREACHABLE; //! TODO
    }
    break;
    case P_TYPE::SCALE: {
        UNREACHABLE; //! TODO
    }
    break;
    case P_TYPE::SPATIUM:   return value<Spatium>().val();
    case P_TYPE::MILIMETRE: return qreal(value<Milimetre>());
    case P_TYPE::PAIR_REAL: return QVariant::fromValue(value<PairF>().toQPairF());

    // draw
    case P_TYPE::COLOR:      return value<draw::Color>().toQColor();
    case P_TYPE::ALIGN:      return QVariant::fromValue(int(value<Align>()));
    case P_TYPE::PLACEMENT:  return static_cast<int>(value<Ms::Placement>());
    case P_TYPE::HPLACEMENT: return static_cast<int>(value<Ms::HPlacement>());
    case P_TYPE::DIRECTION:  return QVariant::fromValue(value<Ms::Direction>());
    case P_TYPE::DIRECTION_H: {
        UNREACHABLE; //! TODO
    }
    break;
    // time
    case P_TYPE::FRACTION:  return QVariant::fromValue(value<Ms::Fraction>());
    case P_TYPE::TDURATION: return QVariant::fromValue(toTDuration());
    // other
    case P_TYPE::BARLINE_TYPE: return static_cast<int>(value<Ms::BarLineType>());
    case P_TYPE::SYMID:        return static_cast<int>(value<Ms::SymId>());
    case P_TYPE::HOOK_TYPE:    return static_cast<int>(value<Ms::HookType>());
    case P_TYPE::DYNAMIC_TYPE: return static_cast<int>(value<Ms::DynamicType>());
    case P_TYPE::ACCIDENTAL_ROLE: return static_cast<int>(value<Ms::AccidentalRole>());
    default:
        UNREACHABLE; //! TODO
    }

    return QVariant();
}

PropertyValue PropertyValue::fromQVariant(const QVariant& v, P_TYPE type)
{
    switch (type) {
    case P_TYPE::UNDEFINED:  // try by QVariant type
        break;

    // Base
    case P_TYPE::BOOL:          return PropertyValue(v.toBool());
    case P_TYPE::INT:           return PropertyValue(v.toInt());
    case P_TYPE::REAL:          return PropertyValue(v.toReal());
    case P_TYPE::STRING:        return PropertyValue(v.toString());

    // Geometry
    case P_TYPE::POINT:         return PropertyValue(PointF::fromQPointF(v.value<QPointF>()));
    case P_TYPE::SIZE:          return PropertyValue(SizeF::fromQSizeF(v.value<QSizeF>()));
    case P_TYPE::PATH: {
        UNREACHABLE; //! TODO
    }
    break;
    case P_TYPE::SCALE:         return PropertyValue(ScaleF::fromQSizeF(v.value<QSizeF>()));
    case P_TYPE::SPATIUM:       return PropertyValue(Spatium(v.toReal()));
    case P_TYPE::MILIMETRE:     return PropertyValue(Milimetre(v.toReal()));
    case P_TYPE::PAIR_REAL:     return PropertyValue(PairF::fromQPairF(v.value<QPair<qreal, qreal> >()));

    // Draw
    case P_TYPE::COLOR:         return PropertyValue(Color::fromQColor(v.value<QColor>()));

    // Layout
    case P_TYPE::ALIGN:         return PropertyValue(Align(v.toInt()));
    case P_TYPE::PLACEMENT:     return PropertyValue(Ms::Placement(v.toInt()));
    case P_TYPE::HPLACEMENT:    return PropertyValue(Ms::HPlacement(v.toInt()));
    case P_TYPE::DIRECTION:     return PropertyValue(Ms::Direction(v.toInt()));
    case P_TYPE::DIRECTION_H: {
        UNREACHABLE; //! TODO
    }
    break;
    // time
    case P_TYPE::FRACTION:      return PropertyValue(v.value<Ms::Fraction>());
    case P_TYPE::TDURATION: {
        UNREACHABLE; //! TODO
    }
    break;
    // other
    case P_TYPE::BARLINE_TYPE: return PropertyValue(Ms::BarLineType(v.toInt()));
    case P_TYPE::SYMID:        return PropertyValue(Ms::SymId(v.toInt()));
    case P_TYPE::HOOK_TYPE:    return PropertyValue(Ms::HookType(v.toInt()));
    case P_TYPE::DYNAMIC_TYPE: return PropertyValue(Ms::DynamicType(v.toInt()));
    case P_TYPE::ACCIDENTAL_ROLE: return PropertyValue(Ms::AccidentalRole(v.toInt()));
    default:
        break;
    }

    //! NOTE Try determinate type by QVariant type
    switch (v.type()) {
    case QVariant::Invalid:     return PropertyValue();
    case QVariant::Bool:        return PropertyValue(v.toBool());
    case QVariant::Int:         return PropertyValue(v.toInt());
    case QVariant::UInt:        return PropertyValue(v.toInt());
    case QVariant::LongLong:    return PropertyValue(v.toInt());
    case QVariant::ULongLong:   return PropertyValue(v.toInt());
    case QVariant::Double:      return PropertyValue(v.toReal());
    case QVariant::Char:        return PropertyValue(v.toInt());
    case QVariant::String:      return PropertyValue(v.toString());
    case QVariant::Size:        return PropertyValue(SizeF::fromQSizeF(QSizeF(v.toSize())));
    case QVariant::SizeF:       return PropertyValue(SizeF::fromQSizeF(v.toSizeF()));
    case QVariant::Point:       return PropertyValue(PointF::fromQPointF(QPointF(v.toPoint())));
    case QVariant::PointF:      return PropertyValue(PointF::fromQPointF(v.toPointF()));
    case QVariant::Color:       return PropertyValue(draw::Color::fromQColor(v.value<QColor>()));
    case QVariant::UserType: {
        const char* type = v.typeName();
        if (strcmp(type, "Ms::Direction") == 0) {
            return PropertyValue(v.value<Ms::Direction>());
        }
        if (strcmp(type, "Ms::Fraction") == 0) {
            return PropertyValue(v.value<Ms::Fraction>());
        }
        LOGE() << "unhandle type: " << type;
        UNREACHABLE;
    }
    break;
    default:
        break;
    }

    UNREACHABLE;
    return PropertyValue();
}
