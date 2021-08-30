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

#ifndef __NOTEDOT_H__
#define __NOTEDOT_H__

#include "element.h"

namespace Ms {
class Note;
class Rest;

//---------------------------------------------------------
//   @@ NoteDot
//---------------------------------------------------------

class NoteDot final : public Element
{
public:
    NoteDot(Note* parent);
    NoteDot(Rest* parent);

    NoteDot* clone() const override { return new NoteDot(*this); }
    qreal mag() const override;

    void draw(mu::draw::Painter*) const override;
    void read(XmlReader&) override;
    void layout() override;

    Note* note() const { return parent()->isNote() ? toNote(parent()) : 0; }
    Rest* rest() const { return parent()->isRest() ? toRest(parent()) : 0; }
    Element* elementBase() const override;
};
}     // namespace Ms
#endif
