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

#ifndef MU_VST_HOSTAPPLICATION_H
#define MU_VST_HOSTAPPLICATION_H

#include "pluginterfaces/vst/ivsthostapplication.h"
#include "base/source/fobject.h"

namespace mu {
namespace vst {
class HostApplication : public Steinberg::FObject, public Steinberg::Vst::IHostApplication
{
public:
    HostApplication();

    //! return host's name
    Steinberg::tresult getName(Steinberg::Vst::String128 name) override;

    Steinberg::tresult createInstance(Steinberg::TUID cid, Steinberg::TUID _iid, void** obj) override;

    //! add base methods
    OBJ_METHODS(HostApplication, Steinberg::FObject)

    //! add methods addRef and release
    REFCOUNT_METHODS(Steinberg::FObject)

    //! add queryInterface(const Steinberg::TUID iid, void** obj) method
    DEFINE_INTERFACES
    DEF_INTERFACE(Steinberg::FObject)
    DEF_INTERFACE(Steinberg::Vst::IHostApplication)
    END_DEFINE_INTERFACES(Steinberg::FObject)
};
}
}
#endif // MU_VST_HOSTAPPLICATION_H
