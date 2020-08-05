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
#ifndef MU_CONTEXT_GLOBALCONTEXT_H
#define MU_CONTEXT_GLOBALCONTEXT_H

#include <vector>

#include "../iglobalcontext.h"
#include "shortcuts/ishortcutcontextresolver.h"
#include "modularity/ioc.h"
#include "scenes/playback/iplaybackcontroller.h"
#include "iinteractive.h"

namespace mu {
namespace context {
class GlobalContext : public IGlobalContext, public shortcuts::IShortcutContextResolver
{
    INJECT(context, framework::IInteractive, interactive)
    INJECT(context, scene::playback::IPlaybackController, playbackController)

public:
    void addMasterNotation(const domain::notation::IMasterNotationPtr& notation) override;
    void removeMasterNotation(const domain::notation::IMasterNotationPtr& notation) override;
    const std::vector<domain::notation::IMasterNotationPtr>& masterNotations() const override;
    bool containsMasterNotation(const io::path& path) const override;

    void setCurrentMasterNotation(const domain::notation::IMasterNotationPtr& notation) override;
    domain::notation::IMasterNotationPtr currentMasterNotation() const override;
    async::Notification currentMasterNotationChanged() const override;

    void setCurrentNotation(const domain::notation::INotationPtr& notation) override;
    domain::notation::INotationPtr currentNotation() const override;
    async::Notification currentNotationChanged() const override;

    shortcuts::ShortcutContext currentShortcutContext() const;

private:
    std::vector<domain::notation::IMasterNotationPtr> m_masterNotations;

    domain::notation::IMasterNotationPtr m_currentMasterNotation;
    async::Notification m_currentMasterNotationChanged;

    domain::notation::INotationPtr m_currentNotation;
    async::Notification m_currentNotationChanged;
};
}
}

#endif // MU_CONTEXT_GLOBALCONTEXT_H
