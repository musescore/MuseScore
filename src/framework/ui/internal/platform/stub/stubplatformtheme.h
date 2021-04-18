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

#ifndef MU_UI_STUBPLATFORMTHEME_H
#define MU_UI_STUBPLATFORMTHEME_H

#include "internal/iplatformtheme.h"

namespace mu::ui {
class StubPlatformTheme : public IPlatformTheme
{
public:
    void startListening() override;
    void stopListening() override;

    bool isFollowSystemThemeAvailable() const override;

    ThemeCode themeCode() const override;
    async::Channel<ThemeCode> themeCodeChanged() const override;

    void applyPlatformStyleOnAppForTheme(ThemeCode themeCode) override;
    void applyPlatformStyleOnWindowForTheme(QWidget* window, ThemeCode themeCode) override;

private:
    async::Channel<ThemeCode> m_channel;
};
}

#endif // MU_UI_STUBPLATFORMTHEME_H
