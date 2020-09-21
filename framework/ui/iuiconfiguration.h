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

#ifndef IUICONFIGURATION_H
#define IUICONFIGURATION_H

#include <QString>

#include "modularity/imoduleexport.h"
#include "async/channel.h"

namespace mu {
namespace framework {
class IUiConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUiConfiguration)

public:
    enum class ThemeType {
        DARK_THEME = 0,
        LIGHT_THEME
    };

    virtual ~IUiConfiguration() = default;

    virtual ThemeType themeType() const = 0;
    virtual async::Channel<ThemeType> themeTypeChanged() = 0;

    virtual QString fontFamily() const = 0;
    virtual async::Channel<QString> fontFamilyChanged() = 0;

    virtual int fontSize() const = 0;
    virtual async::Channel<int> fontSizeChanged() = 0;

    virtual QString musicalFontFamily() const = 0;
    virtual async::Channel<QString> musicalFontFamilyChanged() = 0;

    virtual int musicalFontSize() const = 0;
    virtual async::Channel<int> musicalFontSizeChanged() = 0;

    virtual float guiScaling() const = 0;
};
}
}

#endif // IUICONFIGURATION_H
