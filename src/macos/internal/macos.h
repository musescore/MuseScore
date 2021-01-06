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
#ifndef MU_MACOS_MACOS_H
#define MU_MACOS_MACOS_H

#include "imacos.h"

namespace mu::macos {
class MacOS : public IMacOS
{
public:
    MacOS() = default;

    void init();

    void* observeDistributedNotifications(std::string, std::function<void()>) const override;

    void stopObservingDistributedNotificationsForName(std::string) override;
    void stopObservingDistributedNotificationsForToken(void*) override;
    void stopObservingDistributedAllNotifications() override;

    bool isSystemDarkModeOn() const override;
    async::Channel<bool> systemDarkModeSwitched() const override;
    void setAppAppearanceDark(bool) override;

private:
    async::Channel<bool> m_systemDarkModeSwitched;
};
}

#endif // MU_MACOS_MACOS_H
