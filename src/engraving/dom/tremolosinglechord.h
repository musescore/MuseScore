/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#pragma once

#include "engravingitem.h"

#include "durationtype.h"
#include "draw/types/painterpath.h"
#include "../types/types.h"
#include "beam.h"
#include "chord.h"

namespace mu::engraving {
class Chord;
class TremoloSingleChord final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, TremoloSingleChord)
    DECLARE_CLASSOF(ElementType::TREMOLO_SINGLECHORD)

public:

    TremoloSingleChord& operator=(const TremoloSingleChord&) = delete;
    TremoloSingleChord* clone() const override { return new TremoloSingleChord(*this); }
    void setOwnershipParent(DummyParentOr<Chord> parent) { EngravingItem::setOwnershipParent(parent); }
    ~TremoloSingleChord() override;

    Chord* chord() const { return toChord(ownershipParent()); }

    int subtype() const override { return static_cast<int>(m_tremoloType); }
    TranslatableString subtypeUserName() const override;

    void scanElements(std::function<void(EngravingItem*)> func) override;

    void setTremoloType(TremoloType t);
    TremoloType tremoloType() const { return m_tremoloType; }

    double minHeight() const;

    double chordMag() const;
    RectF drag(EditData&) override;

    TDuration durationType() const;
    void setDurationType(TDuration d);

    Fraction tremoloLen() const;
    bool isBuzzRoll() const { return m_tremoloType == TremoloType::BUZZ_ROLL; }

    int lines() const { return m_lines; }

    staff_idx_t vStaffIdx() const override;
    String accessibleInfo() const override;

    bool playTremolo() const { return m_playTremolo; }
    void setPlayTremolo(bool v) { m_playTremolo = v; }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    bool isMovable() const override { return true; }

    struct LayoutData : public EngravingItem::LayoutData {
    public:
        ld_field<muse::draw::PainterPath> path = { "[TremoloSingleChord] path" };

        bool isValid() const override { return EngravingItem::LayoutData::isValid() && path.has_value(); }
    };
    DECLARE_LAYOUTDATA_METHODS(TremoloSingleChord)

private:
    friend class Factory;

    TremoloSingleChord(DummyParentOr<Chord> parent);
    TremoloSingleChord(const TremoloSingleChord&);

    TremoloType m_tremoloType = TremoloType::INVALID_TREMOLO;
    TDuration m_durationType;
    bool m_playTremolo = true;

    int m_lines = 0; // derived from _subtype
};
} // namespace mu::engraving
