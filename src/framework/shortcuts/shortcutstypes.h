/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_SHORTCUTS_SHORTCUTSTYPES_H
#define MU_SHORTCUTS_SHORTCUTSTYPES_H

#include <string>
#include <list>
#include <QKeySequence>

#include "midi/midievent.h"

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

struct MidiMapping {
    std::string action;
    midi::Event event;

    MidiMapping() = default;
    MidiMapping(const std::string& action)
        : action(action) {}

    bool isValid() const
    {
        return !action.empty() && event;
    }

    bool operator ==(const MidiMapping& mm) const
    {
        return action == mm.action && mm.event == event;
    }
};

using MidiMappingList = std::list<MidiMapping>;
}

#endif // MU_SHORTCUTS_SHORTCUTSTYPES_H
