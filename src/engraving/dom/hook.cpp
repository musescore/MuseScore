/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "hook.h"

#include "style/style.h"

#include "chord.h"
#include "note.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

Hook::Hook(Chord* parent)
    : Symbol(ElementType::HOOK, parent, ElementFlag::NOTHING)
{
    setZ(int(type()) * 100);
}

EngravingItem* Hook::elementBase() const
{
    return parentItem();
}

void Hook::setHookType(int i)
{
    bool straight = style().styleB(Sid::useStraightNoteFlags);
    m_hookType = i;
    setSym(symIdForHookIndex(i, straight));
}

PointF Hook::smuflAnchor() const
{
    return symSmuflAnchor(m_sym, chord()->up() ? SmuflAnchorId::stemUpNW : SmuflAnchorId::stemDownSW);
}

SymId Hook::symIdForHookIndex(int index, bool straight)
{
    switch (index) {
    case 0:
        return SymId::noSym;
    case 1:
        return straight ? SymId::flag8thUpStraight : SymId::flag8thUp;
    case 2:
        return straight ? SymId::flag16thUpStraight : SymId::flag16thUp;
    case 3:
        return straight ? SymId::flag32ndUpStraight : SymId::flag32ndUp;
    case 4:
        return straight ? SymId::flag64thUpStraight : SymId::flag64thUp;
    case 5:
        return straight ? SymId::flag128thUpStraight : SymId::flag128thUp;
    case 6:
        return straight ? SymId::flag256thUpStraight : SymId::flag256thUp;
    case 7:
        return straight ? SymId::flag512thUpStraight : SymId::flag512thUp;
    case 8:
        return straight ? SymId::flag1024thUpStraight : SymId::flag1024thUp;

    case -1:
        return straight ? SymId::flag8thDownStraight : SymId::flag8thDown;
    case -2:
        return straight ? SymId::flag16thDownStraight : SymId::flag16thDown;
    case -3:
        return straight ? SymId::flag32ndDownStraight : SymId::flag32ndDown;
    case -4:
        return straight ? SymId::flag64thDownStraight : SymId::flag64thDown;
    case -5:
        return straight ? SymId::flag128thDownStraight : SymId::flag128thDown;
    case -6:
        return straight ? SymId::flag256thDownStraight : SymId::flag256thDown;
    case -7:
        return straight ? SymId::flag512thDownStraight : SymId::flag512thDown;
    case -8:
        return straight ? SymId::flag1024thDownStraight : SymId::flag1024thDown;
    default:
        LOGE() << "No hook/flag for hook index: " << index;
        break;
    }

    return SymId::noSym;
}

/*!
 * Default hook (flag) properties.
 * For @c Pid::COLOR, returns the sentinel @c configuration()->defaultColor() when
 * @c Sid::colorApplyToFlag is on so that resetting restores the "inherit from note" state;
 * @c Hook::color() resolves the sentinel to the top note's color at draw time.
 */
PropertyValue Hook::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::COLOR:
        if (chord() && !chord()->notes().empty()) {
            if (chord()->upNote()->style().styleV(Sid::colorApplyToFlag).toBool()) {
                return PropertyValue::fromValue(configuration()->defaultColor());
            }
        }
    // fall through
    default:
        return Symbol::propertyDefault(id);
    }
}

/*!
 * Draw color when using the score default: follows the top note's color if flags inherit note color.
 */
Color Hook::color() const
{
    if (m_color == configuration()->defaultColor()) {
        if (chord() && !chord()->notes().empty()) {
            if (chord()->upNote()->style().styleV(Sid::colorApplyToFlag).toBool()) {
                return chord()->upNote()->color();
            }
        }
    }
    return m_color;
}
