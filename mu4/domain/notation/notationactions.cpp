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
#include "notationactions.h"

#include <vector>

using namespace mu::domain::notation;
using namespace mu::actions;

//! NOTO Only actions processed by notation

static const std::vector<Action> _actions = {
    {
        "domain/notation/file-open",
        QT_TRANSLATE_NOOP("action", "Open...")
    },
    {
        "domain/notation/note-input",
        QT_TRANSLATE_NOOP("action", "Note input")
    },
    {
        "domain/notation/pad-note-4",
        QT_TRANSLATE_NOOP("action", "Note 4")
    },
    {
        "domain/notation/pad-note-8",
        QT_TRANSLATE_NOOP("action", "Note 8")
    },
    {
        "domain/notation/pad-note-16",
        QT_TRANSLATE_NOOP("action", "Note 16")
    },
    {
        "domain/notation/put-note", // args: QPoint pos, bool replace, bool insert
        QT_TRANSLATE_NOOP("action", "Put note")
    }
};

const Action& NotationActions::action(const ActionName& name)
{
    for (const Action& a : _actions) {
        if (a.name == name) {
            return a;
        }
    }

    static Action null;
    return null;
}
