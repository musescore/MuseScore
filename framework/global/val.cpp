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
#include "stringutils.h"

using namespace mu;

static const std::string VAL_TRUE("true");
static const std::string VAL_FALSE("false");

Val::Val(const char* str)
    : m_val(str), m_type(Type::String) {}

Val::Val(const std::string& str)
    : m_val(str), m_type(Type::String) {}

Val::Val(std::string&& str)
    : m_val(std::move(str)), m_type(Type::String) {}

Val::Val(double val)
    : m_val(strings::toString(val)), m_type(Type::Double) {}

Val::Val(bool val)
    : m_val(val ? "1" : "0"), m_type(Type::Bool) {}

Val::Val(int val)
    : m_val(strings::toString(val)), m_type(Type::Int) {}

Val::Val(QColor val)
    : m_val(val.name().toStdString()), m_type(Type::Color) {}

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
    return m_val.empty();
}

const std::string& Val::toString()const
{
    if (m_type == Type::Bool) {
        return toBool() ? VAL_TRUE : VAL_FALSE;
    }
    return m_val;
}

double Val::toDouble() const
{
    if (m_val.empty()) {
        return 0.0;
    }

    try {
        return std::stof(m_val);
    } catch (...) {
        return m_val.empty() ? 0.0 : 1.0;
    }
}

bool Val::toBool() const
{
    if (m_val.empty()) {
        return false;
    }

    if (VAL_TRUE == m_val) {
        return true;
    }

    if (VAL_FALSE == m_val) {
        return false;
    }

    try {
        return std::stoi(m_val);
    } catch (...) {
        return m_val.empty() ? false : true;
    }
}

int Val::toInt() const
{
    if (m_val.empty()) {
        return 0;
    }

    try {
        return std::stoi(m_val);
    } catch (...) {
        return m_val.empty() ? 0 : 1;
    }
}

QColor Val::toQColor() const
{
    if (Type::Color == m_type || Type::String == m_type) {
        return QColor(m_val.c_str());
    }
    return QColor();
}

QVariant Val::toQVariant() const
{
    switch (m_type) {
    case Val::Type::Undefined: return QVariant();
    case Val::Type::Bool: return QVariant(toBool());
    case Val::Type::Int: return QVariant(toInt());
    case Val::Type::Double: return QVariant(toDouble());
    case Val::Type::String: return QVariant(QString::fromStdString(toString()));
    case Val::Type::Color: return QVariant::fromValue(toQColor());
    }
    return QVariant();
}

Val Val::fromQVariant(const QVariant& var)
{
    if (!var.isValid()) {
        return Val();
    }

    switch (var.type()) {
    case QVariant::Bool: return Val(var.toBool());
    case QVariant::Int: return Val(var.toInt());
    case QVariant::Double: return Val(var.toDouble());
    case QVariant::String: return Val(var.toString().toStdString());
    case QVariant::Color: return Val(var.value<QColor>());
    default:
        LOGE() << "not supported type: " << var.typeName() << ", val: " << var.toString();
        break;
    }
    return Val();
}
