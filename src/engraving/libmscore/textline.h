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

#ifndef __TEXTLINE_H__
#define __TEXTLINE_H__

#include "textlinebase.h"

namespace mu::engraving {
class Note;

//---------------------------------------------------------
//   @@ TextLineSegment
//---------------------------------------------------------

class TextLineSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, TextLineSegment)

    Sid getTextLinePos(bool above) const;
    Sid getPropertyStyle(Pid) const override;

public:
    TextLineSegment(Spanner* sp, System* parent, bool system=false);

    TextLineSegment* clone() const override { return new TextLineSegment(*this); }

    virtual EngravingItem* propertyDelegate(Pid) override;

    TextLine* textLine() const { return toTextLine(spanner()); }
    void layout() override;
};

//---------------------------------------------------------
//   @@ TextLine
//---------------------------------------------------------

class TextLine final : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, TextLine)
    DECLARE_CLASSOF(ElementType::TEXTLINE)

    Sid getTextLinePos(bool above) const;
    Sid getPropertyStyle(Pid) const override;

public:
    TextLine(EngravingItem* parent, bool system=false);
    TextLine(const TextLine&);
    ~TextLine() {}

    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;

    TextLine* clone() const override { return new TextLine(*this); }

    void initStyle();

    LineSegment* createLineSegment(System* parent) override;
    PropertyValue propertyDefault(Pid) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
};
} // namespace mu::engraving
#endif
