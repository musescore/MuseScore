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

#include <map>
#include "../iglobalcontext.h"

#include "modularity/ioc.h"
#include "shortcuts/ishortcutscontroller.h"
#include "shortcuts/shortcutstypes.h"

namespace mu {
namespace context {
class GlobalContext : public IGlobalContext
{
    INJECT(context, shortcuts::IShortcutsController, shortcutsController)

public:
    GlobalContext() = default;

    void setCurrentNotation(const std::shared_ptr<domain::notation::INotation>& notation) override;
    std::shared_ptr<domain::notation::INotation> currentNotation() const override;
    async::Notification currentNotationChanged() const override;

    bool isPlaying() const override;
    void setIsPlaying(bool arg) override;
    async::Notification isPlayingChanged() const override;

private:

    shortcuts::ShortcutContext currentShortcutContext() const;
    void updateShortcutContext();

    std::shared_ptr<domain::notation::INotation> m_notation;
    async::Notification m_notationChanged;

    bool m_isPlaying = false;
    async::Notification m_isPlayingChanged;
};
}
}

#endif // MU_CONTEXT_GLOBALCONTEXT_H
