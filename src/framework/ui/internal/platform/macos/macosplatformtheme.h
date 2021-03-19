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

#ifndef MU_UI_MACOSPLATFORMTHEME_H
#define MU_UI_MACOSPLATFORMTHEME_H

#include "internal/iplatformtheme.h"

namespace mu::ui {
class MacOSPlatformTheme : public IPlatformTheme
{
public:
    void startListening() override;
    void stopListening() override;

    bool isFollowSystemThemeAvailable() const override;

    bool isDarkMode() const override;
    async::Channel<bool> darkModeSwitched() const override;

    void setAppThemeDark(bool) override;
    void applyPlatformStyle(QWidget*) override;

private:
    async::Channel<bool> m_darkModeSwitched;
};
}

#endif // MU_UI_MACOSPLATFORMTHEME_H
