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

Val::Val(const char* str)
    : val(str), type(Type::String) {}

Val::Val(const std::string& str)
    : val(str), type(Type::String) {}

Val::Val(std::string&& str)
    : val(std::move(str)), type(Type::String) {}

Val::Val(double val)
    : val(std::to_string(val)), type(Type::Double) {}

Val::Val(bool val)
    : val(std::to_string(val ? 1 : 0)), type(Type::Bool) {}

Val::Val(int val)
    : val(std::to_string(val)), type(Type::Int) {}

Val::Val(QColor val)
    : val(val.name().toStdString()), type(Type::Color) {}

bool Val::isNull() const
{
    return val.empty();
}

const std::string& Val::toString()const
{
    return val;
}

double Val::toDouble() const
{
    return std::stof(val);
}

bool Val::toBool() const
{
    if (val == "true") {
        return true;
    }

    if (val == "false") {
        return false;
    }

    return std::stoi(val);
}

int Val::toInt() const
{
    return std::stoi(val);
}

QColor Val::toQColor() const
{
    return QColor(val.c_str());
}

QVariant Val::toQVariant() const
{
    switch (type) {
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
