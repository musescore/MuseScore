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
#ifndef MU_VST_ATTRIBUTELIST_H
#define MU_VST_ATTRIBUTELIST_H

#include <map>
#include <memory>
#include "attributevalue.h"
#include "pluginterfaces/vst/ivstattributes.h"
#include "base/source/fobject.h"

namespace mu {
namespace vst {
class AttributeList : public Steinberg::Vst::IAttributeList
{
public:
    AttributeList();
    virtual ~AttributeList() = default;

    DECLARE_FUNKNOWN_METHODS

    Steinberg::tresult setInt(AttrID aid, Steinberg::int64 value) override;
    Steinberg::tresult getInt(AttrID aid, Steinberg::int64& value) override;
    Steinberg::tresult setFloat(AttrID aid, double value) override;
    Steinberg::tresult getFloat(AttrID aid, double& value) override;
    Steinberg::tresult setString(AttrID aid, const Steinberg::Vst::TChar* string) override;
    Steinberg::tresult getString(AttrID aid, Steinberg::Vst::TChar* string, Steinberg::uint32 sizeInBytes) override;
    Steinberg::tresult setBinary(AttrID aid, const void* data, Steinberg::uint32 sizeInBytes) override;
    Steinberg::tresult getBinary(AttrID aid, const void*& data, Steinberg::uint32& sizeInBytes) override;

protected:
    void removeAttrID(AttrID aid);
    std::map<AttrID, std::unique_ptr<AttributeValue> > m_list = {};
};
}
}

#endif // MU_VST_ATTRIBUTELIST_H
