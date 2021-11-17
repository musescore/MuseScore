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
    : m_type(Ms::P_TYPE::BOOL), m_val(v)
{
}

PropertyValue::PropertyValue(int v)
    : m_type(Ms::P_TYPE::INT), m_val(v)
{
}

PropertyValue::PropertyValue(qreal v)
    : m_type(Ms::P_TYPE::REAL), m_val(v)
{
}

PropertyValue::PropertyValue(const char* v)
    : m_type(Ms::P_TYPE::STRING), m_val(QString(v))
{
}

PropertyValue::PropertyValue(const QString& v)
    : m_type(Ms::P_TYPE::STRING), m_val(v)
{
}

PropertyValue::PropertyValue(const Ms::Spatium& v)
    : m_type(Ms::P_TYPE::SPATIUM), m_val(v)
{
}

PropertyValue::PropertyValue(const PointF& v)
    : m_type(Ms::P_TYPE::POINT), m_val(v)
{
}

PropertyValue::PropertyValue(const SizeF& v)
    : m_type(Ms::P_TYPE::SIZE), m_val(v)
{
}

PropertyValue::PropertyValue(const PainterPath& v)
    : m_type(Ms::P_TYPE::PATH), m_val(v)
{
}

PropertyValue::PropertyValue(const draw::Color& v)
    : m_type(Ms::P_TYPE::COLOR), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::Align v)
    : m_type(Ms::P_TYPE::ALIGN), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::Direction v)
    : m_type(Ms::P_TYPE::DIRECTION), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::SymId v)
    : m_type(Ms::P_TYPE::SYMID), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::BarLineType v)
    : m_type(Ms::P_TYPE::BARLINE_TYPE), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::HookType v)
    : m_type(Ms::P_TYPE::HOOK_TYPE), m_val(v)
{
}

PropertyValue::PropertyValue(Ms::HPlacement v)
    : m_type(Ms::P_TYPE::HPLACEMENT), m_val(v)
{
}

PropertyValue::PropertyValue(const Ms::PitchValues& v)
    : m_type(Ms::P_TYPE::PITCH_VALUES), m_val(v)
{
}

PropertyValue::PropertyValue(const Ms::Groups& v)
    : m_type(Ms::P_TYPE::GROUPS), m_any(v)
{
}

const Ms::Groups& PropertyValue::toGroups() const
{
    return std::any_cast<const Ms::Groups&>(m_any);
}

PropertyValue::PropertyValue(const Ms::TDuration& v)
    : m_type(Ms::P_TYPE::TDURATION), m_any(v)
{
}

const Ms::TDuration& PropertyValue::toTDuration() const
{
    return std::any_cast<const Ms::TDuration&>(m_any);
}

PropertyValue::PropertyValue(const Ms::Fraction& v)
    : m_type(Ms::P_TYPE::FRACTION), m_val(v)
{
}

PropertyValue::PropertyValue(const QList<int>& v)
    : m_type(Ms::P_TYPE::INT_LIST), m_val(v)
{
}

bool PropertyValue::isValid() const
{
    return m_type != Ms::P_TYPE::UNDEFINED;
}

Ms::P_TYPE PropertyValue::type() const
{
    return m_type;
}

bool PropertyValue::operator ==(const PropertyValue& v) const
{
    //! HACK Temporary hack for bool comparisons (maybe one type is bool and another type is int)
    if (v.m_type == Ms::P_TYPE::BOOL || m_type == Ms::P_TYPE::BOOL) {
        return v.value<bool>() == value<bool>();
    }

    //! HACK Temporary hack for Spatium comparisons (maybe one type is Spatium and another type is real)
    if (v.m_type == Ms::P_TYPE::SPATIUM || m_type == Ms::P_TYPE::SPATIUM) {
        return RealIsEqual(v.value<qreal>(), value<qreal>());
    }

    //! HACK Temporary hack for Fraction comparisons
    if (v.m_type == Ms::P_TYPE::FRACTION) {
        assert(m_type == Ms::P_TYPE::FRACTION);
        return v.value<Ms::Fraction>().identical(value<Ms::Fraction>());
    }

    //! HACK Temporary hack for TDURATION comparisons
    if (v.m_type == Ms::P_TYPE::TDURATION) {
        assert(m_type == Ms::P_TYPE::TDURATION);
        return v.toTDuration() == toTDuration();
    }

    //! HACK Temporary hack for GROUPS comparisons
    if (v.m_type == Ms::P_TYPE::GROUPS) {
        assert(m_type == Ms::P_TYPE::GROUPS);
        return v.toGroups() == toGroups();
    }

    if (v.m_type == Ms::P_TYPE::REAL) {
        assert(m_type == Ms::P_TYPE::REAL);
        return RealIsEqual(v.value<qreal>(), value<qreal>());
    }

    return v.m_type == m_type && v.m_val == m_val;
}

QVariant PropertyValue::toQVariant() const
{
    switch (m_type) {
    case Ms::P_TYPE::UNDEFINED: return QVariant();
    // base
    case Ms::P_TYPE::BOOL:      return value<bool>();
    case Ms::P_TYPE::INT:       return value<int>();
    case Ms::P_TYPE::REAL:      return value<qreal>();
    case Ms::P_TYPE::STRING:    return value<QString>();
    // geometry
    case Ms::P_TYPE::POINT:     return value<PointF>().toQPointF();
    case Ms::P_TYPE::SIZE:      return value<SizeF>().toQSizeF();
    case Ms::P_TYPE::PATH: {
        UNREACHABLE; //! TODO
    }
    break;
    case Ms::P_TYPE::SCALE: {
        UNREACHABLE; //! TODO
    }
    break;
    case Ms::P_TYPE::SPATIUM:   return value<Ms::Spatium>().val();
    case Ms::P_TYPE::SIZE_MM: {
        UNREACHABLE; //! TODO
    }
    break;
    case Ms::P_TYPE::POINT_SP: {
        UNREACHABLE; //! TODO
    }
    break;
    case Ms::P_TYPE::POINT_MM: {
        UNREACHABLE; //! TODO
    }
    break;
    case Ms::P_TYPE::POINT_SP_MM: {
        UNREACHABLE; //! TODO
    }
    break;
    case Ms::P_TYPE::SP_REAL: {
        UNREACHABLE; //! TODO
    }
    break;
    // draw
    case Ms::P_TYPE::COLOR:     return value<draw::Color>().toQColor();
    case Ms::P_TYPE::FONT: {
        UNREACHABLE; //! TODO
    }
    break;
    case Ms::P_TYPE::ALIGN:      return QVariant::fromValue(value<Ms::Align>());
    case Ms::P_TYPE::PLACEMENT:  return static_cast<int>(value<Ms::Placement>());
    case Ms::P_TYPE::HPLACEMENT: return static_cast<int>(value<Ms::HPlacement>());
    case Ms::P_TYPE::DIRECTION:  return value<SizeF>().toQSizeF();
    case Ms::P_TYPE::DIRECTION_H: {
        UNREACHABLE; //! TODO
    }
    break;
    // time
    case Ms::P_TYPE::FRACTION:  return QVariant::fromValue(value<Ms::Fraction>());

    default:
        UNREACHABLE; //! TODO
    }
    return QVariant();
}

PropertyValue PropertyValue::fromQVariant(const QVariant& v)
{
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
        if (strcmp(type, "Ms::Spatium") == 0) {
            return PropertyValue(v.value<Ms::Spatium>());
        }
        if (strcmp(type, "Ms::Direction") == 0) {
            return PropertyValue(v.value<Ms::Direction>());
        }
        if (strcmp(type, "Ms::Align") == 0) {
            return PropertyValue(v.value<Ms::Align>());
        }
        if (strcmp(type, "mu::draw::Color") == 0) {
            return PropertyValue(v.value<draw::Color>());
        }
        if (strcmp(type, "mu::SizeF") == 0) {
            return PropertyValue(v.value<mu::SizeF>());
        }
        if (strcmp(type, "mu::PointF") == 0) {
            return PropertyValue(v.value<PointF>());
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
