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

#ifndef MU_ENGRAVING_PICKSCRAPE_H
#define MU_ENGRAVING_PICKSCRAPE_H

#include "chordtextlinebase.h"

namespace mu::engraving {
class PickScrape;

//---------------------------------------------------------
//   @@ PickScrapeSegment
//---------------------------------------------------------

class PickScrapeSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, PickScrapeSegment)
    DECLARE_CLASSOF(ElementType::PICK_SCRAPE_SEGMENT)

public:
    PickScrapeSegment(PickScrape* sp, System* parent);

    PickScrapeSegment* clone() const override { return new PickScrapeSegment(*this); }

    PickScrape* pickScrape() const { return (PickScrape*)spanner(); }

    friend class PickScrape;
};

//---------------------------------------------------------
//   @@ PickScrape
//---------------------------------------------------------

class PickScrape final : public ChordTextLineBase
{
    OBJECT_ALLOCATOR(engraving, PickScrape)
    DECLARE_CLASSOF(ElementType::PICK_SCRAPE)

public:
    PickScrape(EngravingItem* parent);

    PickScrape* clone() const override { return new PickScrape(*this); }

    LineSegment* createLineSegment(System* parent) override;

    PropertyValue propertyDefault(Pid propertyId) const override;
    Sid getPropertyStyle(Pid) const override;
};
} // namespace mu::engraving
#endif
