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

#ifndef MU_ENGRAVING_NOTELINE_H
#define MU_ENGRAVING_NOTELINE_H

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

public:
    NoteLine(EngravingItem* parent);
    NoteLine(const NoteLine&);
    ~NoteLine() {}

    NoteLine* clone() const override { return new NoteLine(*this); }

    void setStartNote(Note* n) { m_startNote = n; }
    Note* startNote() const { return m_startNote; }
    void setEndNote(Note* n) { m_endNote = n; }
    Note* endNote() const { return m_endNote; }
    LineSegment* createLineSegment(System* parent) override;

private:

    Note* m_startNote = nullptr;
    Note* m_endNote = nullptr;
};
} // namespace mu::engraving
#endif
