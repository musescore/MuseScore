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
#pragma once

#include "slur.h"
#include "textbase.h"

namespace mu::engraving {
class HammerOnPullOffText final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, HammerOnPullOffText)
    DECLARE_CLASSOF(ElementType::HAMMER_ON_PULL_OFF_TEXT)

public:
    HammerOnPullOffText(HammerOnPullOffSegment* parent = nullptr);
    HammerOnPullOffText(const HammerOnPullOffText& h);
    HammerOnPullOffText* clone() const override { return new HammerOnPullOffText(*this); }

    bool isEditAllowed(EditData&) const override { return false; }

    Chord* startChord() const { return m_startChord; }
    Chord* endChord() const { return m_endChord; }
    void setStartChord(Chord* c) { m_startChord = c; }
    void setEndChord(Chord* c) { m_endChord = c; }

    std::vector<LineF> dragAnchorLines() const override;

    bool isUserModified() const override;
    bool isValid() const { return m_isValid; }
    void setIsValid(bool v) { m_isValid = v; }
    bool isHammerOn() const { return m_isHammerOn; }
    void setIsHammerOn(bool v) { m_isHammerOn = v; }

    Color curColor() const override;

private:
    Chord* m_startChord = nullptr;
    Chord* m_endChord = nullptr;
    bool m_isValid = true;
    bool m_isHammerOn = true;
};

class HammerOnPullOffSegment final : public SlurSegment
{
    OBJECT_ALLOCATOR(engraving, HammerOnPullOffSegment)
    DECLARE_CLASSOF(ElementType::HAMMER_ON_PULL_OFF_SEGMENT)

public:
    HammerOnPullOffSegment(System* parent);
    HammerOnPullOffSegment(const HammerOnPullOffSegment& other);

    HammerOnPullOffSegment* clone() const override { return new HammerOnPullOffSegment(*this); }

    HammerOnPullOff* hammerOnPullOff() const { return toHammerOnPullOff(spanner()); }

    Color curColor() const override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    EngravingObjectList scanChildren() const override;

    void setTrack(track_idx_t idx) override;

    void updateHopoText();
    void addHopoText(HammerOnPullOffText* t) { m_hopoText.push_back(t); }
    const std::vector<HammerOnPullOffText*>& hopoText() const { return m_hopoText; }

    bool isUserModified() const override;
    bool isValid() const;

    void reset() override;

private:
    struct HopoTextRegion {
        Chord* startChord = nullptr;
        Chord* endChord = nullptr;
        bool isHammerOn = true;
        bool isValid = true;
    };

    std::vector<HopoTextRegion> computeHopoTextRegions(Chord* startChord, Chord* endChord);

    std::vector<HammerOnPullOffText*> m_hopoText;
};

class HammerOnPullOff final : public Slur
{
    OBJECT_ALLOCATOR(engraving, HammerOnPullOff)
    DECLARE_CLASSOF(ElementType::HAMMER_ON_PULL_OFF)

public:
    HammerOnPullOff* clone() const override { return new HammerOnPullOff(*this); }
    SlurTieSegment* newSlurTieSegment(System* parent) override { return new HammerOnPullOffSegment(parent); }

    friend class Factory;

    HammerOnPullOff(EngravingItem* parent);
    HammerOnPullOff(const HammerOnPullOff&);
};
} // namespace mu::engraving
