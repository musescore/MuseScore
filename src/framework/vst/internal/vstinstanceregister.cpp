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
#include "vstinstanceregister.h"

using namespace mu::vst;

VSTInstanceRegister::VSTInstanceRegister()
{
}

unsigned int VSTInstanceRegister::count()
{
    return m_instances.size();
}

instanceId VSTInstanceRegister::addInstance(instancePtr instance)
{
    auto it = m_instances.insert(m_instances.end(), instance);
    return std::distance(m_instances.begin(), it);
}

instancePtr VSTInstanceRegister::instance(instanceId id)
{
    if (id != IVSTInstanceRegister::ID_NOT_SETTED
        && id < static_cast<int>(m_instances.size())) {
        return m_instances[id];
    }
    return nullptr;
}
