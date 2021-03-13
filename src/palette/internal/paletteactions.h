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
#ifndef MU_PALETTE_PALETTEACTIONS_H
#define MU_PALETTE_PALETTEACTIONS_H

#include "actions/imoduleactions.h"

namespace mu::palette {
class PaletteActions : public actions::IModuleActions
{
public:
    const actions::ActionItem& action(const actions::ActionCode& actionCode) const override;

    static const actions::ActionCodeList actionCodes(shortcuts::ShortcutContext context);

private:
    static const std::vector<actions::ActionItem> m_actions;
};
}

#endif // MU_NOTATION_NOTATIONACTIONS_H
