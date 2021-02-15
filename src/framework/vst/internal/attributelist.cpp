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

#include "attributelist.h"
#include "base/source/fstring.h"
using namespace mu::vst;
using namespace Steinberg;
using namespace Steinberg::Vst;

IMPLEMENT_FUNKNOWN_METHODS(AttributeList, IAttributeList, IAttributeList::iid)
DEF_CLASS_IID(IAttributeList)

AttributeList::AttributeList()
{
}

tresult AttributeList::setInt(IAttributeList::AttrID aid, int64 value)
{
    m_list[aid] = std::unique_ptr<AttributeValue>(new AttributeValue(value));
    return kResultOk;
}

tresult AttributeList::getInt(IAttributeList::AttrID aid, int64& value)
{
    if (m_list.find(aid) == m_list.end()) {
        return kResultFalse;
    }

    value = m_list[aid]->intValue();
    return kResultOk;
}

tresult AttributeList::setFloat(IAttributeList::AttrID aid, double value)
{
    m_list[aid] = std::unique_ptr<AttributeValue>(new AttributeValue(value));
    return kResultOk;
}

tresult AttributeList::getFloat(IAttributeList::AttrID aid, double& value)
{
    if (m_list.find(aid) == m_list.end()) {
        return kResultFalse;
    }

    value = m_list[aid]->floatValue();
    return kResultOk;
}

tresult AttributeList::setString(IAttributeList::AttrID aid, const TChar* string)
{
    m_list[aid] = std::unique_ptr<AttributeValue>(new AttributeValue(string, String(const_cast<TChar*>(string)).length() + 1));
    return kResultOk;
}

tresult AttributeList::getString(IAttributeList::AttrID aid, TChar* string, uint32 sizeInBytes)
{
    if (m_list.find(aid) == m_list.end()) {
        return kResultFalse;
    }

    uint32 stringSize = 0;
    const TChar* _string = m_list[aid]->stringValue(stringSize);
    memcpy(string, _string, std::min<uint32>(stringSize * sizeof(Steinberg::Vst::TChar), sizeInBytes));
    return kResultOk;
}

tresult AttributeList::setBinary(IAttributeList::AttrID aid, const void* data, uint32 sizeInBytes)
{
    m_list[aid] = std::unique_ptr<AttributeValue>(new AttributeValue(data, sizeInBytes));
    return kResultOk;
}

tresult AttributeList::getBinary(IAttributeList::AttrID aid, const void*& data, uint32& sizeInBytes)
{
    if (m_list.find(aid) == m_list.end()) {
        return kResultFalse;
    }

    data = m_list[aid]->binaryValue(sizeInBytes);
    return kResultOk;
}

void AttributeList::removeAttrID(IAttributeList::AttrID aid)
{
    m_list.erase(aid);
}
