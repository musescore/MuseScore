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

namespace mu {
namespace framework {
class UiConfiguration : public IUiConfiguration
{
public:
    UiConfiguration();

    ThemeType themeType() const override;
    async::Channel<ThemeType> themeTypeChanged() override;

    QString fontFamily() const override;
    async::Channel<QString> fontFamilyChanged() override;

    int fontSize() const override;
    async::Channel<int> fontSizeChanged() override;

private:

    async::Channel<ThemeType> m_currentThemeTypeChannel;
    async::Channel<QString> m_currentFontFamilyChannel;
    async::Channel<int> m_currentFontSizeChannel;
};
}
}

#endif // UICONFIGURATION_H
