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

#include "ret.h"

using namespace mu;

Ret::Ret(bool arg)
    : m_code(arg ? int(Code::Ok) : int(Code::UnknownError))
{}

Ret::Ret(int c)
    : m_code(c)
{}

Ret::Ret(Code c)
    : m_code(static_cast<int>(c))
{
}

Ret::Ret(const int& c, const std::string& text)
    : m_code(c), m_text(text)
{}

bool Ret::valid() const
{
    return m_code > int(Code::Undefined);
}

bool Ret::success() const
{
    return m_code == int(Code::Ok);
}

void Ret::setCode(int c)
{
    m_code = c;
}

int Ret::code() const
{
    return m_code;
}

void Ret::setText(const std::string& s)
{
    m_text = s;
}

const std::string& Ret::text() const
{
    return m_text;
}

std::string Ret::toString() const
{
    return "[" + std::to_string(m_code) + "] " + m_text;
}
