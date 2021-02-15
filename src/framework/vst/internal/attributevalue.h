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
#ifndef MU_VST_ATTRIBUTEVALUE_H
#define MU_VST_ATTRIBUTEVALUE_H

#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace mu {
namespace vst {
class AttributeValue
{
public:
    enum Type
    {
        Integer,
        Float,
        String,
        Binary
    };
    AttributeValue();
    AttributeValue(Steinberg::int64 value);
    AttributeValue(double value);
    AttributeValue(const Steinberg::Vst::TChar* value, Steinberg::uint32 m_size);
    AttributeValue(const void* value, Steinberg::uint32 m_size);

    ~AttributeValue();

    Steinberg::int64 intValue() const;
    double floatValue() const;
    const Steinberg::Vst::TChar* stringValue(Steinberg::uint32& stringSize);
    const void* binaryValue(Steinberg::uint32& binarySize);

    Type getType() const;

protected:
    union
    {
        Steinberg::int64 intValue = 0;
        double floatValue;
        Steinberg::Vst::TChar* stringValue;
        char* binaryValue;
    } m_value;

    Steinberg::uint32 m_size = 0;
    Type m_type = Integer;
};
}
}

#endif // MU_VST_ATTRIBUTEVALUE_H
