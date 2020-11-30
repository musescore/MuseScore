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
#include "async/notification.h"

namespace mu::framework {
class IUiConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IUiConfiguration)

public:
    virtual ~IUiConfiguration() = default;

    enum class ThemeType {
        DARK_THEME = 0,
        LIGHT_THEME
    };

    virtual ThemeType themeType() const = 0;
    virtual async::Channel<ThemeType> themeTypeChanged() const = 0;

    enum class FontSizeType {
        BODY,
        BODY_LARGE,
        TABS,
        HEADER,
        TITLE
    };

    virtual std::string fontFamily() const = 0;
    virtual int fontSize(FontSizeType type) const = 0;
    virtual async::Notification fontChanged() const = 0;

    virtual std::string iconsFontFamily() const = 0;
    virtual int iconsFontSize() const = 0;
    virtual async::Notification iconsFontChanged() const = 0;

    virtual std::string musicalFontFamily() const = 0;
    virtual int musicalFontSize() const = 0;
    virtual async::Notification musicalFontChanged() const = 0;

    virtual float guiScaling() const = 0;
    virtual float physicalDotsPerInch() const = 0;
};
}

#endif // IUICONFIGURATION_H
