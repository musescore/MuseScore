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

bool PropertyValue::isValid() const
{
    return m_type != Ms::P_TYPE::UNDEFINED;
}

Ms::P_TYPE PropertyValue::type() const
{
    return m_type;
}

QVariant PropertyValue::toQVariant() const
{
    if (const bool* pv = std::get_if<bool>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const int* pv = std::get_if<int>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const qreal* pv = std::get_if<qreal>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const QString* pv = std::get_if<QString>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const Ms::Spatium* pv = std::get_if<Ms::Spatium>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const PointF* pv = std::get_if<PointF>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const SizeF* pv = std::get_if<SizeF>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const draw::Color* pv = std::get_if<draw::Color>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const Ms::Align* pv = std::get_if<Ms::Align>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    if (const Ms::Direction* pv = std::get_if<Ms::Direction>(&m_val)) {
        return QVariant::fromValue(*pv);
    }
    return QVariant();
}

PropertyValue PropertyValue::fromQVariant(const QVariant& v)
{
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
    if (strcmp(type, "Ms::Spatium") == 0) {
        return PropertyValue(v.value<Ms::Spatium>());
    }
    if (strcmp(type, "QString") == 0) {
        return PropertyValue(v.value<QString>());
    }
    if (strcmp(type, "qreal") == 0) {
        return PropertyValue(v.value<qreal>());
    }
    if (strcmp(type, "double") == 0) {
        return PropertyValue(qreal(v.value<double>()));
    }
    if (strcmp(type, "float") == 0) {
        return PropertyValue(qreal(v.value<float>()));
    }
    if (strcmp(type, "int") == 0) {
        return PropertyValue(v.value<int>());
    }
    if (strcmp(type, "bool") == 0) {
        return PropertyValue(v.value<bool>());
    }
    LOGE() << "unhandle type: " << type;
    UNREACHABLE;
    return PropertyValue();
}
