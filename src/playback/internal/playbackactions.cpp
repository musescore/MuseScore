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
#include "playbackactions.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::shortcuts;

//! NOTE Only actions processed by notation

const mu::actions::ActionList PlaybackActions::m_actions = {
    ActionItem("play",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Play")
               ),
};

const ActionItem& PlaybackActions::action(const ActionCode& actionCode) const
{
    for (const ActionItem& a : m_actions) {
        if (a.code == actionCode) {
            return a;
        }
    }

    static ActionItem null;
    return null;
}
