//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "val.h"
#include "log.h"

using namespace mu;

static const std::string VAL_TRUE("true");
static const std::string VAL_FALSE("false");

Val::Val(const char* str)
    : m_val(str), m_type(Type::String) {}

Val::Val(const std::string& str)
    : m_val(QString::fromStdString(str)), m_type(Type::String) {}

Val::Val(std::string&& str)
    : m_val(QString::fromStdString(str)), m_type(Type::String) {}

Val::Val(double val)
    : m_val(val), m_type(Type::Double) {}

Val::Val(bool val)
    : m_val(val), m_type(Type::Bool) {}

Val::Val(int val)
    : m_val(val), m_type(Type::Int) {}

Val::Val(QColor color)
    : m_val(std::move(color)), m_type(Type::Color) {}

Val::Val(QVariant val)
    : m_val(std::move(val)), m_type(Type::Variant) {}

void Val::setType(Type t)
{
    m_type = t;
}

Val::Type Val::type() const
{
    return m_type;
}

bool Val::isNull() const
{
    return m_val.isNull();
}

std::string Val::toString()const
{
    if (m_type == Type::Bool) {
        return toBool() ? VAL_TRUE : VAL_FALSE;
    }
    return m_val.toString().toStdString();
}

double Val::toDouble() const
{
    return m_val.toDouble();
}

float Val::toFloat() const
{
    return m_val.toFloat();
}

bool Val::toBool() const
{
    if (m_val.isNull()) {
        return false;
    }

    std::string stdStr = m_val.toString().toStdString();

    if (VAL_TRUE == stdStr) {
        return true;
    }

    if (VAL_FALSE == stdStr) {
        return false;
    }

    try {
        return std::stoi(stdStr);
    } catch (...) {
        return m_val.isNull() ? false : true;
    }
}

int Val::toInt() const
{
    return m_val.toInt();
}

QColor Val::toQColor() const
{
    return m_val.value<QColor>();
}

QString Val::toQString() const
{
    return QString::fromStdString(toString());
}

QVariant Val::toQVariant() const
{
    switch (m_type) {
    case Val::Type::Undefined: return QVariant();
    case Val::Type::Bool: return QVariant(toBool());
    case Val::Type::Int: return QVariant(toInt());
    case Val::Type::Double: return QVariant(toDouble());
    case Val::Type::String: return QVariant(QString::fromStdString(toString()));
    case Val::Type::Color:
    case Val::Type::Variant: return m_val;
    }
    return QVariant();
}

Val Val::fromQVariant(const QVariant& var)
{
    switch (var.type()) {
    case QVariant::Bool: return Val(var.toBool());
    case QVariant::Int: return Val(var.toInt());
    case QVariant::Double: return Val(var.toDouble());
    case QVariant::String: return Val(var.toString().toStdString());
    default: return Val(var);
    }
}

bool Val::operator ==(const Val& v) const
{
    return v.m_val == m_val && v.m_type == m_type;
}

bool Val::operator <(const Val& v) const
{
    switch (v.type()) {
    case Type::Bool: return toBool() < v.toBool();
    case Type::Int: return toInt() < v.toInt();
    case Type::Double: return toDouble() < v.toDouble();
    case Type::String: return toString() < v.toString();
    default: return toString() < v.toString();
    }
}
