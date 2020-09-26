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
#ifndef MU_RPC_RPCTYPES_H
#define MU_RPC_RPCTYPES_H

#include <vector>
#include <string>

namespace mu {
namespace rpc {
struct Variable
{
    Variable()
        : m_type(""), m_data({ 0 }) {}

    Variable(bool value)
        : m_type(typeid(value).name())
    {
        m_data.d_bool = value;
    }

    Variable(int value)
        : m_type(typeid(value).name())
    {
        m_data.d_int = value;
    }

    Variable(unsigned int value)
        : m_type(typeid(value).name())
    {
        m_data.d_uint = value;
    }

    Variable(unsigned long value)
        : m_type(typeid(value).name())
    {
        m_data.d_ulong = value;
    }

    Variable(float value)
        : m_type(typeid(value).name())
    {
        m_data.d_float = value;
    }

    Variable(double value)
        : m_type(typeid(value).name())
    {
        m_data.d_double = value;
    }

    bool toBool() const { return m_data.d_bool; }
    int toInt() const { return m_data.d_int; }
    unsigned int toUInt() const { return m_data.d_uint; }
    unsigned long toULong() const { return m_data.d_ulong; }
    float toFloat() const { return m_data.d_float; }
    double toDouble() const { return m_data.d_double; }

    std::string type() const { return m_type; }

private:
    const char* m_type;
    union {
        bool d_bool;
        int d_int;
        unsigned int d_uint;
        unsigned long d_ulong;
        float d_float;
        double d_double;
    } m_data;
};

using ArgumentList = std::vector<Variable>;

using target_id = std::pair<std::string, unsigned int>;

struct Message
{
    Message(target_id id, std::string procedure, ArgumentList arguments)
        : target(id), method(procedure), args(arguments) {}

    target_id target;
    std::string method;
    ArgumentList args;
};
}
}
#endif // MU_RPC_RPCTYPES_H
