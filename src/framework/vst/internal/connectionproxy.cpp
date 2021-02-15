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
#include "connectionproxy.h"

using namespace mu::vst;
using namespace Steinberg;
using namespace Vst;

DEF_CLASS_IID(IConnectionPoint)

ConnectionProxy::ConnectionProxy(IConnectionPoint* source)
    : m_source(source), m_destination(nullptr)
{
}

tresult ConnectionProxy::connect(IConnectionPoint* destination)
{
    if (!destination) {
        return kInvalidArgument;
    }

    if (m_destination) {
        return kResultFalse;
    }

    tresult res = m_source->connect(this);
    if (res != kResultTrue) {
        m_destination = nullptr;
    } else {
        m_destination = destination;
    }
    return res;
}

tresult ConnectionProxy::disconnect(IConnectionPoint* destination)
{
    if (!destination) {
        return kInvalidArgument;
    }

    if (destination != m_destination) {
        return kInvalidArgument;
    }

    if (m_source) {
        m_source->disconnect(this);
    }
    m_destination = nullptr;
    return kResultOk;
}

tresult ConnectionProxy::notify(IMessage* message)
{
    if (m_destination) {
        if (m_threadID == std::this_thread::get_id()) {
            return m_destination->notify(message);
        }
    }
    return kResultFalse;
}
