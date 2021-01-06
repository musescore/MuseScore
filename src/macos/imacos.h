//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_MACOS_IMACOS_H
#define MU_MACOS_IMACOS_H

#include "modularity/imoduleexport.h"
#include "async/channel.h"

namespace mu::macos {
class IMacOS : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMacOS)
public:
    virtual ~IMacOS() = default;

    virtual void* observeDistributedNotifications(std::string, std::function<void()>) const = 0;

    virtual void stopObservingDistributedNotificationsForName(std::string) = 0;
    virtual void stopObservingDistributedNotificationsForToken(void*) = 0;
    virtual void stopObservingDistributedAllNotifications() = 0;

    virtual bool isSystemDarkModeOn() const = 0;
    virtual async::Channel<bool> systemDarkModeSwitched() const = 0;
    virtual void setAppAppearanceDark(bool) = 0;
};
}

#endif // MU_MACOS_IMACOS_H
