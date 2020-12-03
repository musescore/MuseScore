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

#ifndef IINSPECTORADAPTER_H
#define IINSPECTORADAPTER_H

#include <QVariant>

#include "modularity/imoduleexport.h"

#include "libmscore/style.h"
#include "async/notification.h"

namespace Ms {
class Score;
}

namespace mu {
namespace inspector {
class IInspectorAdapter : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IInspectorAdapter)
public:
    virtual ~IInspectorAdapter() = default;

    virtual bool isNotationExisting() const = 0;
    virtual bool isTextEditingStarted() const = 0;
    virtual async::Notification isTextEditingChanged() const = 0;

    // notation commands
    virtual void beginCommand() = 0;
    virtual void endCommand() = 0;

    // notation styling
    virtual void updateStyleValue(const Ms::Sid& styleId, const QVariant& newValue) = 0;
    virtual QVariant styleValue(const Ms::Sid& styleId) = 0;

    // dialogs
    virtual void showSpecialCharactersDialog() = 0;
    virtual void showStaffTextPropertiesDialog() = 0;
    virtual void showPageSettingsDialog() = 0;
    virtual void showStyleSettingsDialog() = 0;
    virtual void showTimeSignaturePropertiesDialog() = 0;
    virtual void showArticulationPropertiesDialog() = 0;
    virtual void showGridConfigurationDialog() = 0;

    // actions
    virtual void updatePageMarginsVisibility(const bool isVisible) = 0;
    virtual void updateFramesVisibility(const bool isVisible) = 0;
    virtual void updateHorizontalGridSnapping(const bool isSnapped) = 0;
    virtual void updateVerticalGridSnapping(const bool isSnapped) = 0;
    virtual void updateUnprintableElementsVisibility(const bool isVisible) = 0;
    virtual void updateInvisibleElementsDisplaying(const bool isVisible) = 0; //!Note Invisible elements can be displayed as a semi-transparent elements

    // notation layout
    virtual void updateNotation() = 0;
};
}
}

#endif // IINSPECTORADAPTER_H
