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
#ifndef MU_APPSHELL_NOTATIONPAGESTATE_H
#define MU_APPSHELL_NOTATIONPAGESTATE_H

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "inotationpagestate.h"
#include "iappshellconfiguration.h"

namespace mu::appshell {
class NotationPageState : public INotationPageState, public async::Asyncable
{
    INJECT(appshell, IAppShellConfiguration, configuration)

public:
    void init();

    bool isPanelVisible(PanelType type) const override;
    void setIsPanelsVisible(const std::map<PanelType, bool>& panelsVisible) override;
    mu::async::Channel<PanelTypeList> panelsVisibleChanged() const override;

private:
    void setIsPanelVisible(PanelType type, bool visible);

    mutable std::map<PanelType, bool> m_panelVisibleMap;
    async::Channel<PanelTypeList> m_panelsVisibleChanged;
};
}

#endif // MU_APPSHELL_NOTATIONPAGESTATE_H
