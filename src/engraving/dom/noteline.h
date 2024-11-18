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

#ifndef MU_ENGRAVING_NOTELINE_H
#define MU_ENGRAVING_NOTELINE_H

#include "textlinebase.h"

namespace mu::engraving {
class NoteLineSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, NoteLineSegment)
    DECLARE_CLASSOF(ElementType::NOTELINE_SEGMENT)

    Sid getPropertyStyle(Pid) const override;

public:
    NoteLineSegment(Spanner* sp, System* parent);

    NoteLine* noteLine() const { return toNoteLine(spanner()); }

    NoteLineSegment* clone() const override { return new NoteLineSegment(*this); }

    EngravingItem* propertyDelegate(Pid) override;
};

class NoteLine final : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, NoteLine)
    DECLARE_CLASSOF(ElementType::NOTELINE)

    Sid getPropertyStyle(Pid) const override;

public:
    NoteLine(EngravingItem* parent);
    NoteLine(const NoteLine&);
    ~NoteLine() {}

    NoteLine* clone() const override { return new NoteLine(*this); }

    LineSegment* createLineSegment(System* parent) override;

    PropertyValue propertyDefault(Pid) const override;
    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;

    bool allowTimeAnchor() const override { return false; }

    NoteLineEndPlacement lineEndPlacement() { return m_lineEndPlacement; }
    void setLineEndPlacement(NoteLineEndPlacement v) { m_lineEndPlacement = v; }

    void reset() override;

    bool enforceMinLength() { return m_lineEndPlacement != NoteLineEndPlacement::LEFT_EDGE; }
private:
    NoteLineEndPlacement m_lineEndPlacement = NoteLineEndPlacement::OFFSET_ENDS;
};
} // namespace mu::engraving
#endif
