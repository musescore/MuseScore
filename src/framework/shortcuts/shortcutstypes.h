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
#ifndef MU_SHORTCUTS_SHORTCUTSTYPES_H
#define MU_SHORTCUTS_SHORTCUTSTYPES_H

#include <string>
#include <list>
#include <QKeySequence>

namespace mu::shortcuts {
struct Shortcut
{
    std::string action;
    std::string sequence;
    QKeySequence::StandardKey standardKey = QKeySequence::UnknownKey;

    bool isValid() const
    {
        return !action.empty() && (!sequence.empty() || standardKey != QKeySequence::UnknownKey);
    }

    bool operator ==(const Shortcut& sc) const
    {
        return action == sc.action && sequence == sc.sequence && standardKey == sc.standardKey;
    }
};

using ShortcutList = std::list<Shortcut>;
}

#endif // MU_SHORTCUTS_SHORTCUTSTYPES_H
