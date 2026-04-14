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

#ifndef MU_ENGRAVING_NOTEDOT_H
#define MU_ENGRAVING_NOTEDOT_H

#include "engravingitem.h"

namespace mu::engraving {
class Factory;
class Note;
class Rest;

/*!
 * Augmentation dot attached to a @c Note or @c Rest; shares placement and style with its parent.
 */
class NoteDot final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, NoteDot)
    DECLARE_CLASSOF(ElementType::NOTEDOT)

public:

    //! @return Heap copy of this dot (palette / editing).
    NoteDot* clone() const override { return new NoteDot(*this); }
    //! @return Product of the parent element’s scale and @c Sid::dotMag.
    double mag() const override;

    //! @return Parent as @c Note, or @c nullptr if parent is a rest.
    Note* note() const { return explicitParent()->isNote() ? toNote(explicitParent()) : 0; }
    //! @return Parent as @c Rest, or @c nullptr if parent is a note.
    Rest* rest() const { return explicitParent()->isRest() ? toRest(explicitParent()) : 0; }
    //! @return The parent note or rest (see @c parentItem()).
    EngravingItem* elementBase() const override;

    //! Default @c Pid::COLOR: parent @c Note::getProperty / @c Rest default when dots inherit themed color.
    PropertyValue propertyDefault(Pid propertyId) const override;
    //! Draw color: parent @c Note::color() or rest equivalent when inheriting.
    Color color() const override;

private:
    friend class Factory;
    //! Constructed only via @c Factory; attaches to @p parent note.
    NoteDot(Note* parent);
    //! Constructed only via @c Factory; attaches to @p parent rest.
    NoteDot(Rest* parent);
};
} // namespace mu::engraving
#endif
