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

#ifndef __NOTELINE_H__
#define __NOTELINE_H__

#include "textlinebase.h"

namespace mu::engraving {
class Note;

//---------------------------------------------------------
//   @@ NoteLine
//---------------------------------------------------------

class NoteLine final : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, NoteLine)
    DECLARE_CLASSOF(ElementType::NOTELINE)

    Note* _startNote;
    Note* _endNote;

public:
    NoteLine(EngravingItem* parent);
    NoteLine(const NoteLine&);
    ~NoteLine() {}

    NoteLine* clone() const override { return new NoteLine(*this); }

    void setStartNote(Note* n) { _startNote = n; }
    Note* startNote() const { return _startNote; }
    void setEndNote(Note* n) { _endNote = n; }
    Note* endNote() const { return _endNote; }
    LineSegment* createLineSegment(System* parent) override;
};
} // namespace mu::engraving
#endif
