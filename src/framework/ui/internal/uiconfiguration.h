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
#include "iplatformtheme.h"

#include "modularity/ioc.h"

namespace mu::framework {
class UiConfiguration : public IUiConfiguration
{
    INJECT(framework, IMainWindow, mainWindow)
    INJECT(framework, IPlatformTheme, platformTheme)

public:
    void init();

    ThemeType preferredThemeType() const override;
    async::Channel<ThemeType> preferredThemeTypeChanged() const override;
    ThemeType actualThemeType() const override;
    async::Channel<ThemeType> actualThemeTypeChanged() const override;

    std::string fontFamily() const override;
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

    void setPhysicalDotsPerInch(std::optional<float> dpi) override;

private:
    async::Channel<ThemeType> m_currentPreferredThemeTypeChannel;
    async::Channel<ThemeType> m_currentActualThemeTypeChannel;

    async::Notification m_fontChanged;
    async::Notification m_musicalFontChanged;
    async::Notification m_iconsFontChanged;

    std::optional<float> m_customDPI;
};
}

#endif // UICONFIGURATION_H
