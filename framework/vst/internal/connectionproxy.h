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
#ifndef MU_VST_CONNECTIONPROXY_H
#define MU_VST_CONNECTIONPROXY_H

#include <thread>
#include "base/source/fobject.h"
#include "pluginterfaces/vst/ivstmessage.h"

namespace mu {
namespace vst {
class ConnectionProxy : public Steinberg::FObject, public Steinberg::Vst::IConnectionPoint
{
public:
    ConnectionProxy (Steinberg::Vst::IConnectionPoint* source);
    ~ConnectionProxy () override = default;

    //basic methods for FObject
    OBJ_METHODS(ConnectionProxy, Steinberg::FObject)
    REFCOUNT_METHODS(Steinberg::FObject)
    DEF_INTERFACES_1(Steinberg::Vst::IConnectionPoint, Steinberg::FObject)

    //virtual methods from Steinberg::Vst::IConnectionPoint
    Steinberg::tresult connect(Steinberg::Vst::IConnectionPoint* destination) override;
    Steinberg::tresult disconnect(Steinberg::Vst::IConnectionPoint* destination) override;
    Steinberg::tresult notify(Steinberg::Vst::IMessage* message) override;

private:
    Steinberg::IPtr<Steinberg::Vst::IConnectionPoint> m_source;
    Steinberg::IPtr<Steinberg::Vst::IConnectionPoint> m_destination;

    std::thread::id m_threadID = std::this_thread::get_id();
};
}
}
#endif // MU_VST_CONNECTIONPROXY_H
