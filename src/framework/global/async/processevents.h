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
#ifndef MU_ASYNC_PROCESSEVENTS_H
#define MU_ASYNC_PROCESSEVENTS_H

#include "thirdparty/deto_async/async/channel.h"
namespace mu {
namespace async {
inline void processEvents()
{
    deto::async::processEvents();
}

inline void onMainThreadInvoke(const std::function<void(const std::function<void()>&)>& f)
{
    deto::async::onMainThreadInvoke(f);
}
}
}

#endif // MU_ASYNC_PROCESSEVENTS_H
