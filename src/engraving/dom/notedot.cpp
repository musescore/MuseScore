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

#include "notedot.h"

#include "style/style.h"

#include "note.h"
#include "rest.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
/*!
 * Constructs a dot attached to @p parent note; the dot is not user-movable.
 */
NoteDot::NoteDot(Note* parent)
    : EngravingItem(ElementType::NOTEDOT, parent)
{
    setFlag(ElementFlag::MOVABLE, false);
}

/*!
 * Constructs a dot attached to @p parent rest; the dot is not user-movable.
 */
NoteDot::NoteDot(Rest* parent)
    : EngravingItem(ElementType::NOTEDOT, parent)
{
    setFlag(ElementFlag::MOVABLE, false);
}

/*!
 * @return The parent note or rest (same as @c parentItem()).
 */
EngravingItem* NoteDot::elementBase() const
{
    return parentItem();
}

/*!
 * @return Scale factor from the parent element times @c Sid::dotMag.
 */
double NoteDot::mag() const
{
    return parentItem()->mag() * style().styleD(Sid::dotMag);
}

/*!
 * Default note-dot properties.
 * For @c Pid::COLOR, when @c Sid::colorApplyToDot is enabled, returns the parent
 * note's themed rendering color via @c Note::color() so dots follow the active coloring scheme.
 */
PropertyValue NoteDot::propertyDefault(Pid propertyId) const
{
    if (propertyId == Pid::COLOR) {
        if (note()) {
            if (note()->style().styleV(Sid::colorApplyToDot).toBool()) {
                return PropertyValue::fromValue(note()->color());
            }
        } else if (rest()) {
            if (rest()->style().styleV(Sid::colorApplyToDot).toBool()) {
                return rest()->getProperty(Pid::COLOR);
            }
        }
    }
    return EngravingItem::propertyDefault(propertyId);
}

/*!
 * Draw color when using the score default: if dots inherit themed colors, returns the parent
 * @c Note::color() or @c Rest::color().
 */
Color NoteDot::color() const
{
    if (m_color == configuration()->defaultColor()) {
        if (note()) {
            if (note()->style().styleV(Sid::colorApplyToDot).toBool()) {
                return note()->color();
            }
        } else if (rest()) {
            if (rest()->style().styleV(Sid::colorApplyToDot).toBool()) {
                return rest()->color();
            }
        }
    }
    return m_color;
}
}
