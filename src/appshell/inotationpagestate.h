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
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_APPSHELL_INOTATIONPAGESTATE_H
#define MU_APPSHELL_INOTATIONPAGESTATE_H

#include "modularity/imoduleexport.h"
#include "appshelltypes.h"
#include "async/channel.h"

namespace mu::appshell {
class INotationPageState : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(INotationPageState)

public:
    virtual ~INotationPageState() = default;

    virtual bool isPanelVisible(PanelType type) const = 0;
    virtual void setIsPanelsVisible(const std::map<PanelType, bool>& panelsVisible) = 0;
    virtual async::Channel<PanelTypeList> panelsVisibleChanged() const = 0;
};
}

#endif // MU_APPSHELL_NOTATIONPAGESTATE_H
