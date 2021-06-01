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

#ifndef __STICKING_H__
#define __STICKING_H__

#include "textbase.h"

namespace Ms {
//-----------------------------------------------------------------------------
//   @@ Sticking
///    Drum sticking
//-----------------------------------------------------------------------------

class Sticking final : public TextBase
{
    QVariant propertyDefault(Pid id) const override;

public:
    Sticking(Score*);

    Sticking* clone() const override { return new Sticking(*this); }
    ElementType type() const override { return ElementType::STICKING; }

    Segment* segment() const { return (Segment*)parent(); }
    Measure* measure() const { return (Measure*)parent()->parent(); }

    void layout() override;
    void write(XmlWriter& xml) const override;
    void read(XmlReader&) override;
};
}     // namespace Ms
#endif
