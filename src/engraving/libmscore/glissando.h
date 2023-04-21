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

#ifndef __GLISSANDO_H__
#define __GLISSANDO_H__

#include "engravingitem.h"
#include "line.h"
#include "property.h"
#include "types.h"

namespace mu::engraving {
// the amount of white space to leave before a system-initial chord with glissando
static const double GLISS_STARTOFSYSTEM_WIDTH = 4;           // in sp

class Glissando;
class Note;
enum class GlissandoType;

//---------------------------------------------------------
//   @@ GlissandoSegment
//---------------------------------------------------------

class GlissandoSegment final : public LineSegment
{
    OBJECT_ALLOCATOR(engraving, GlissandoSegment)
public:
    GlissandoSegment(Glissando* sp, System* parent);

    Glissando* glissando() const { return toGlissando(spanner()); }

    GlissandoSegment* clone() const override { return new GlissandoSegment(*this); }
    void draw(mu::draw::Painter*) const override;
    void layout() override;

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
    M_PROPERTY(String, fontFace, setFontFace)
    M_PROPERTY(double, fontSize, setFontSize)
    M_PROPERTY(bool, showText, setShowText)
    M_PROPERTY(bool, playGlissando, setPlayGlissando)
    M_PROPERTY(FontStyle, fontStyle, setFontStyle)
    M_PROPERTY(int, easeIn, setEaseIn)
    M_PROPERTY(int, easeOut, setEaseOut)

public:
    Glissando(EngravingItem* parent);
    Glissando(const Glissando&);

    static Note* guessInitialNote(Chord* chord);
    static Note* guessFinalNote(Chord* chord, Note* startNote);

    const TranslatableString& glissandoTypeName() const;

    // overridden inherited methods
    Glissando* clone() const override { return new Glissando(*this); }

    LineSegment* createLineSegment(System* parent) override;

    void layout() override;

    // property/style methods
    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
    void addLineAttachPoints();

    static bool pitchSteps(const Spanner* spanner, std::vector<int>& pitchOffsets);
};
} // namespace mu::engraving

#endif
