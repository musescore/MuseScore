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
#ifndef MU_NOTATION_NOTATIONINPUTSTATE_H
#define MU_NOTATION_NOTATIONINPUTSTATE_H

#include <map>
#include "../inotationinputstate.h"

#include "igetscore.h"

namespace Ms {
class Score;
}

namespace mu {
namespace notation {
class NotationInputState : public INotationInputState
{
public:
    NotationInputState(IGetScore* getScore);

    bool isNoteEnterMode() const override;
    DurationType duration() const override;

private:

    Ms::Score* score() const;

    IGetScore* m_getScore = nullptr;
};
}
}

#endif // MU_NOTATION_NOTATIONINPUTSTATE_H
