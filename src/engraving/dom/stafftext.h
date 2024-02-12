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

#ifndef MU_ENGRAVING_STAFFTEXT_H
#define MU_ENGRAVING_STAFFTEXT_H

#include "stafftextbase.h"

namespace mu::engraving {
//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

class StaffText final : public StaffTextBase
{
    OBJECT_ALLOCATOR(engraving, StaffText)
    DECLARE_CLASSOF(ElementType::STAFF_TEXT)

public:
    StaffText(Segment* parent = nullptr, TextStyleType = TextStyleType::STAFF);

    StaffText* clone() const override { return new StaffText(*this); }

    bool canBeExcludedFromOtherParts() const override { return true; }

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    EngravingObjectList scanChildren() const override;

    void add(EngravingItem*) override;
    void remove(EngravingItem*) override;

    void setTrack(track_idx_t idx) override;

    bool hasSoundFlag() const;
    SoundFlag* soundFlag() const;
    void setSoundFlag(SoundFlag* flag);

private:
    PropertyValue propertyDefault(Pid id) const override;

    SoundFlag* m_soundFlag = nullptr;
};
} // namespace mu::engraving
#endif
