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

#ifndef UICONFIGURATION_H
#define UICONFIGURATION_H

#include "iuiconfiguration.h"
#include "imainwindow.h"

#include "modularity/ioc.h"

namespace mu::framework {
class UiConfiguration : public IUiConfiguration
{
    INJECT(framework, IMainWindow, mainWindow)

public:
    void init();

    ThemeType themeType() const override;
    async::Channel<ThemeType> themeTypeChanged() const override;

    std::string fontFamily() const override;
    std::string semiBoldFontFamily() const override;
    int fontSize(FontSizeType type) const override;
    async::Notification fontChanged() const override;

    std::string iconsFontFamily() const override;
    int iconsFontSize() const override;
    async::Notification iconsFontChanged() const override;

    std::string musicalFontFamily() const override;
    int musicalFontSize() const override;
    async::Notification musicalFontChanged() const override;

    float guiScaling() const override;
    float physicalDotsPerInch() const override;

private:
    async::Channel<ThemeType> m_currentThemeTypeChannel;

    async::Notification m_fontChanged;
    async::Notification m_musicalFontChanged;
    async::Notification m_iconsFontChanged;
};
}

#endif // UICONFIGURATION_H
