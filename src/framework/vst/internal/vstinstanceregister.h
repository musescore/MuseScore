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
#ifndef MU_VST_VSTSTANCEREGISTER_H
#define MU_VST_VSTSTANCEREGISTER_H

#include "ivstinstanceregister.h"
#include "internal/plugininstance.h"

namespace mu {
namespace vst {
class VSTInstanceRegister : public IVSTInstanceRegister
{
public:
    VSTInstanceRegister();

    unsigned int count() override;
    instanceId addInstance(instancePtr instance) override;
    instancePtr instance(instanceId id) override;

private:
    std::vector<instancePtr> m_instances = {};
};
} // namespace vst
} // namespace mu

#endif // MU_VST_VSTSTANCEREGISTER_H
