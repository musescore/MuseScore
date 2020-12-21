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

#ifndef MU_FRAMEWORK_ITHEME_H
#define MU_FRAMEWORK_ITHEME_H

#include <QColor>
#include <QFont>

#include "modularity/imoduleexport.h"
#include "async/notification.h"

namespace mu::framework {
class ITheme : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ITheme)

public:
    virtual ~ITheme() = default;

    virtual QColor backgroundPrimaryColor() const = 0;
    virtual QColor backgroundSecondaryColor() const = 0;
    virtual QColor popupBackgroundColor() const = 0;
    virtual QColor textFieldColor() const = 0;
    virtual QColor accentColor() const = 0;
    virtual QColor strokeColor() const = 0;
    virtual QColor buttonColor() const = 0;
    virtual QColor fontPrimaryColor() const = 0;
    virtual QColor fontSecondaryColor() const = 0;

    virtual QFont bodyFont() const = 0;
    virtual QFont bodyBoldFont() const = 0;
    virtual QFont largeBodyFont() const = 0;
    virtual QFont largeBodyBoldFont() const = 0;
    virtual QFont tabFont() const = 0;
    virtual QFont tabBoldFont() const = 0;
    virtual QFont headerFont() const = 0;
    virtual QFont headerBoldFont() const = 0;
    virtual QFont titleBoldFont() const = 0;

    virtual QFont iconsFont() const = 0;
    virtual QFont musicalFont() const = 0;

    virtual qreal accentOpacityNormal() const = 0;
    virtual qreal accentOpacityHover() const = 0;
    virtual qreal accentOpacityHit() const = 0;

    virtual qreal buttonOpacityNormal() const = 0;
    virtual qreal buttonOpacityHover() const = 0;
    virtual qreal buttonOpacityHit() const = 0;

    virtual qreal itemOpacityDisabled() const = 0;

    virtual async::Notification themeChanged() const = 0;
};
}

#endif // MU_FRAMEWORK_ITHEME_H
