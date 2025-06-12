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

#include "engraving/types/types.h"

#include "articulation.h"
#include "slur.h"
#include "textbase.h"

namespace mu::engraving {
class TappingHalfSlur;
class TappingText;

class Tapping : public Articulation
{
    OBJECT_ALLOCATOR(engraving, Tapping)
    DECLARE_CLASSOF(ElementType::TAPPING)

public:
    Tapping(const Tapping&);
    Tapping* clone() const override { return new Tapping(*this); }

    struct LayoutData : public Articulation::LayoutData
    {
    };
    DECLARE_LAYOUTDATA_METHODS(Tapping)

    TappingHand hand() const { return m_hand; }
    void setHand(TappingHand h) { m_hand = h; }

    TappingText* text() const { return m_text; }
    void setText(TappingText* text) { m_text = text; }

    void setSelected(bool f) override;

    TappingHalfSlur* halfSlurAbove() const { return m_halfSlurAbove; }
    void setHalfSlurAbove(TappingHalfSlur* s) { m_halfSlurAbove = s; }
    TappingHalfSlur* halfSlurBelow() const { return m_halfSlurBelow; }
    void setHalfSlurBelow(TappingHalfSlur* s) { m_halfSlurBelow = s; }

    LHTappingShowItems lhShowItems() const;

    void styleChanged() override;

    double mag() const override;

    int subtype() const override;

    String accessibleInfo() const override;
    TranslatableString typeUserName() const override;
    TranslatableString subtypeUserName() const override;

protected:
    friend class mu::engraving::Factory;
    Tapping(ChordRest* parent);

private:
    TappingHand m_hand = TappingHand::INVALID;
    TappingText* m_text = nullptr;
    TappingHalfSlur* m_halfSlurAbove = nullptr;
    TappingHalfSlur* m_halfSlurBelow = nullptr;
};

class TappingHalfSlurSegment : public SlurSegment
{
    OBJECT_ALLOCATOR(engraving, TappingHalfSlurSegment)
    DECLARE_CLASSOF(ElementType::TAPPING_HALF_SLUR_SEGMENT)

public:
    TappingHalfSlurSegment(System* parent);
    TappingHalfSlurSegment(const TappingHalfSlurSegment& other);
    TappingHalfSlurSegment* clone() const override { return new TappingHalfSlurSegment(*this); }

    TappingHalfSlur* tappingHalfSlur() const { return toTappingHalfSlur(spanner()); }
};

class TappingHalfSlur : public Slur
{
    OBJECT_ALLOCATOR(engraving, TappingHalfSlur)
    DECLARE_CLASSOF(ElementType::TAPPING_HALF_SLUR)

    bool isHalfSlurAbove() const { return m_isHalfSlurAbove; }
    void setIsHalfSlurAbove(bool v) { m_isHalfSlurAbove = v; }

public:
    TappingHalfSlur(EngravingItem* parent);
    TappingHalfSlur(const TappingHalfSlur&);
    TappingHalfSlur* clone() const override { return new TappingHalfSlur(*this); }
    TappingHalfSlurSegment* newSlurTieSegment(System* parent) override { return new TappingHalfSlurSegment(parent); }

    Tapping* tapping() const { return toTapping(parent()); }

private:
    bool m_isHalfSlurAbove = true;
};

class TappingText final : public TextBase
{
    OBJECT_ALLOCATOR(engraving, TappingText)
    DECLARE_CLASSOF(ElementType::TAPPING_TEXT)

public:
    TappingText(Tapping* parent = nullptr);
    TappingText(const TappingText& t);
    TappingText* clone() const override { return new TappingText(*this); }

    virtual Color curColor() const;
};
} // namespace mu::engraving
