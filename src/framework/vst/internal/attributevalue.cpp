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
#include "attributevalue.h"

#include <cstring>

using namespace mu::vst;

AttributeValue::AttributeValue()
    : m_size(0), m_type(Integer)
{
    m_value.intValue = 0;
}

AttributeValue::AttributeValue(Steinberg::int64 value)
    : m_size(0), m_type(Integer)
{
    m_value.intValue = value;
}

AttributeValue::AttributeValue(double value)
    : m_size(0), m_type(Float)
{
    m_value.floatValue = value;
}

AttributeValue::AttributeValue(const Steinberg::Vst::TChar* value, Steinberg::uint32 size)
    : m_size(size), m_type(String)
{
    m_value.stringValue = new Steinberg::Vst::TChar[size];
    memcpy(m_value.stringValue, value, size * sizeof(Steinberg::Vst::TChar));
}

AttributeValue::AttributeValue(const void* value, Steinberg::uint32 size)
    : m_size(size), m_type(Binary)
{
    m_value.binaryValue = new char[size];
    memcpy(m_value.binaryValue, value, size);
}

AttributeValue::~AttributeValue()
{
    if (m_size) {
        delete[] m_value.binaryValue;
    }
}

Steinberg::int64 AttributeValue::intValue() const
{
    return m_value.intValue;
}

double AttributeValue::floatValue() const
{
    return m_value.floatValue;
}

const Steinberg::Vst::TChar* AttributeValue::stringValue(Steinberg::uint32& stringSize)
{
    stringSize = m_size;
    return m_value.stringValue;
}

const void* AttributeValue::binaryValue(Steinberg::uint32& binarySize)
{
    binarySize = m_size;
    return m_value.binaryValue;
}

AttributeValue::Type AttributeValue::getType() const
{
    return m_type;
}
