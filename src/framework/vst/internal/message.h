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

#ifndef MU_VST_MESSAGE_H
#define MU_VST_MESSAGE_H

#include <string>
#include "pluginterfaces/vst/ivstmessage.h"
#include "attributelist.h"

namespace mu {
namespace vst {
class Message : public Steinberg::Vst::IMessage
{
public:
    Message();
    virtual ~Message() = default;

    DECLARE_FUNKNOWN_METHODS

    const char* getMessageID() override;
    void setMessageID(const char* messageID) override;

    //! returns mu::vst::AttributeList
    Steinberg::Vst::IAttributeList* getAttributes() override;

protected:
    std::string m_messageId = "";
    AttributeList m_attributeList;
};
}
}
#endif // MU_VST_MESSAGE_H
