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

#ifndef MU_ENGRAVING_GLISSANDO_H
#define MU_ENGRAVING_GLISSANDO_H

#include "engravingitem.h"
#include "line.h"
#include "property.h"
#include "types.h"

namespace mu::engraving {
class Glissando;
class Note;
enum class GlissandoType;

//---------------------------------------------------------
//   @@ GlissandoSegment
//---------------------------------------------------------

class GlissandoSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, GlissandoSegment)
    DECLARE_CLASSOF(ElementType::GLISSANDO_SEGMENT)

public:
    GlissandoSegment(Glissando* sp, System* parent);

    Glissando* glissando() const { return toGlissando(spanner()); }

    GlissandoSegment* clone() const override { return new GlissandoSegment(*this); }

    EngravingItem* propertyDelegate(Pid) override;
};

//---------------------------------------------------------
//   Glissando
//---------------------------------------------------------

class Glissando final : public SLine
{
    OBJECT_ALLOCATOR(engraving, Glissando)
    DECLARE_CLASSOF(ElementType::GLISSANDO)

    M_PROPERTY(String, text, setText)
    M_PROPERTY(GlissandoType, glissandoType, setGlissandoType)
    M_PROPERTY(GlissandoStyle, glissandoStyle, setGlissandoStyle)
    M_PROPERTY(bool, glissandoShift, setGlissandoShift)
    M_PROPERTY(String, fontFace, setFontFace)
    M_PROPERTY(double, fontSize, setFontSize)
    M_PROPERTY(bool, showText, setShowText)
    M_PROPERTY(FontStyle, fontStyle, setFontStyle)
    M_PROPERTY(int, easeIn, setEaseIn)
    M_PROPERTY(int, easeOut, setEaseOut)

public:
    static constexpr double GLISS_PALETTE_WIDTH = 4.0;
    static constexpr double GLISS_PALETTE_HEIGHT = 4.0;

    Glissando(EngravingItem* parent);
    Glissando(const Glissando&);

    static Note* guessInitialNote(Chord* chord);

    const TranslatableString& glissandoTypeName() const;

    std::optional<bool> isHarpGliss() const { return m_isHarpGliss; }
    void setIsHarpGliss(std::optional<bool> v) { m_isHarpGliss = v; }

    // overridden inherited methods
    Glissando* clone() const override { return new Glissando(*this); }

    LineSegment* createLineSegment(System* parent) override;

    bool allowTimeAnchor() const override { return false; }

    // property/style methods
    Sid getPropertyStyle(Pid id) const override;
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    static bool pitchSteps(const Spanner* spanner, std::vector<int>& pitchOffsets);

private:

    std::optional<bool> m_isHarpGliss = std::nullopt;
};
} // namespace mu::engraving

#endif
