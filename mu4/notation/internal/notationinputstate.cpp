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
#include "notationinputstate.h"

#include "libmscore/score.h"
#include "libmscore/input.h"

using namespace mu::notation;

NotationInputState::NotationInputState(const IGetScore* getScore)
    : m_getScore(getScore)
{
}

bool NotationInputState::isNoteEnterMode() const
{
    return score()->inputState().noteEntryMode();
}

bool NotationInputState::isPadActive(Pad pad) const
{
    switch (pad) {
    case Pad::NOTE00: return isDurationActive(DurationType::V_LONG);
    case Pad::NOTE0: return isDurationActive(DurationType::V_BREVE);
    case Pad::NOTE1: return isDurationActive(DurationType::V_WHOLE);
    case Pad::NOTE2: return isDurationActive(DurationType::V_HALF);
    case Pad::NOTE4: return isDurationActive(DurationType::V_QUARTER);
    case Pad::NOTE8: return isDurationActive(DurationType::V_EIGHTH);
    case Pad::NOTE16: return isDurationActive(DurationType::V_16TH);
    case Pad::NOTE32: return isDurationActive(DurationType::V_32ND);
    case Pad::NOTE64: return isDurationActive(DurationType::V_64TH);
    case Pad::NOTE128: return isDurationActive(DurationType::V_128TH);
    case Pad::NOTE256: return isDurationActive(DurationType::V_256TH);
    case Pad::NOTE512: return isDurationActive(DurationType::V_512TH);
    case Pad::NOTE1024: return isDurationActive(DurationType::V_1024TH);
    case Pad::REST: /*todo*/ return false;
    case Pad::DOT: /*todo*/ return false;
    case Pad::DOTDOT: /*todo*/ return false;
    case Pad::DOT3: /*todo*/ return false;
    case Pad::DOT4: /*todo*/ return false;
    }

    return false;
}

Duration NotationInputState::duration() const
{
    return score()->inputState().duration();
}

Ms::Score* NotationInputState::score() const
{
    return m_getScore->score();
}

bool NotationInputState::isDurationActive(DurationType durationType) const
{
    return duration() == durationType;
}
