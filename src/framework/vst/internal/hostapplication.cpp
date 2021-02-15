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

#include "hostapplication.h"

#include "base/source/fstring.h"
#include "pluginterfaces/vst/ivstmessage.h"

#include "message.h"

using namespace mu::vst;
using namespace Steinberg;
using namespace Steinberg::Vst;

DEF_CLASS_IID(IHostApplication)

HostApplication::HostApplication()
{
}

tresult HostApplication::getName(Vst::String128 name)
{
    String str("MuseScore");
    str.copyTo16(name, 0, 127);
    return kResultTrue;
}

tresult HostApplication::createInstance(TUID cid, TUID _iid, void** obj)
{
    FUID classID(FUID::fromTUID(cid));
    FUID interfaceID(FUID::fromTUID(_iid));
    if (classID == IMessage::iid && interfaceID == IMessage::iid) {
        *obj = new Message();
        return kResultTrue;
    } else if (classID == IAttributeList::iid && interfaceID == IAttributeList::iid) {
        *obj = new AttributeList();
        return kResultTrue;
    }
    *obj = nullptr;
    return kResultFalse;
}
