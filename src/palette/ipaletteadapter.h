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
#ifndef MU_PALETTE_IPALETTEADAPTER_H
#define MU_PALETTE_IPALETTEADAPTER_H

#include "modularity/imoduleexport.h"

#include <QString>

#include "retval.h"

#include "async/notification.h"
#include "ui/uitypes.h"

namespace Ms {
class Element;
class PaletteWorkspace;
}

namespace mu::palette {
//! NOTE This is an adapter for the palette,
//! that to use the palette in the MU3 and MU4 build.
class IPaletteAdapter : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IPaletteAdapter)

public:
    virtual ~IPaletteAdapter() = default;

    virtual const ui::UiAction& getAction(const actions::ActionCode& code) const = 0;

    virtual void showMasterPalette(const QString&) = 0;

    // score
    virtual bool isSelected() const = 0;
    virtual bool applyPaletteElement(Ms::Element* element, Qt::KeyboardModifiers modifiers = {}) = 0;

    // qml
    virtual Ms::PaletteWorkspace* paletteWorkspace() const = 0;
    virtual mu::ValCh<bool> paletteEnabled() const = 0;
    virtual void setPaletteEnabled(bool arg) = 0;
    virtual void requestPaletteSearch() = 0;
    virtual async::Notification paletteSearchRequested() const = 0;
    virtual void notifyElementDraggedToScoreView() = 0;
    virtual async::Notification elementDraggedToScoreView() const = 0;
};
}

#endif // MU_PALETTE_IPALETTEADAPTER_H
