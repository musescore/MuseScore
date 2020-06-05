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
#ifndef MU_DOMAIN_NOTATIONINPUTSTATE_H
#define MU_DOMAIN_NOTATIONINPUTSTATE_H

#include <map>
#include "../inotationinputstate.h"

namespace Ms {
class MasterScore;
}

namespace mu {
namespace domain {
namespace notation {
class NotationInputState : public INotationInputState
{
public:
    NotationInputState(Ms::MasterScore* score);

    deto::async::Notify inputStateChanged() const override;
    void notifyAboutISChanged();

    bool noteEntryMode() const override;
    DurationType duration() const override;

private:

    Ms::MasterScore* m_score = nullptr;
    deto::async::Notify m_inputStateChanged;
};
}
}
}

#endif // MU_DOMAIN_NOTATIONINPUTSTATE_H
