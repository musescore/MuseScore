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

#ifndef MU_ENGRAVING_OTTAVA_H
#define MU_ENGRAVING_OTTAVA_H

#include "types/translatablestring.h"

#include "textlinebase.h"
#include "property.h"

namespace mu::engraving {
//---------------------------------------------------------
//   OttavaE
//---------------------------------------------------------

struct OttavaE {
    int offset = 0;
    unsigned start = 0;
    unsigned end = 0;
};

//---------------------------------------------------------
//   OttavaType
//---------------------------------------------------------

enum class OttavaType : unsigned char {
    OTTAVA_8VA,
    OTTAVA_8VB,
    OTTAVA_15MA,
    OTTAVA_15MB,
    OTTAVA_22MA,
    OTTAVA_22MB
};

//---------------------------------------------------------
//   OttavaDefault
//---------------------------------------------------------

struct OttavaDefault {
    OttavaType type = OttavaType::OTTAVA_8VA;
    int shift = 0;
    const char* name = nullptr;
    const TranslatableString userName = TranslatableString();
};

// order is important, should be the same as OttavaType
static const OttavaDefault ottavaDefault[] = {
    { OttavaType::OTTAVA_8VA,  12,  "8va",  TranslatableString("engraving/ottavatype", "8va alta") },
    { OttavaType::OTTAVA_8VB,  -12, "8vb",  TranslatableString("engraving/ottavatype", "8va bassa") },
    { OttavaType::OTTAVA_15MA, 24,  "15ma", TranslatableString("engraving/ottavatype", "15ma alta") },
    { OttavaType::OTTAVA_15MB, -24, "15mb", TranslatableString("engraving/ottavatype", "15ma bassa") },
    { OttavaType::OTTAVA_22MA, 36,  "22ma", TranslatableString("engraving/ottavatype", "22ma alta") },
    { OttavaType::OTTAVA_22MB, -36, "22mb", TranslatableString("engraving/ottavatype", "22ma bassa") }
};

class Ottava;

//---------------------------------------------------------
//   @@ OttavaSegment
//---------------------------------------------------------

class OttavaSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, OttavaSegment)
    DECLARE_CLASSOF(ElementType::OTTAVA_SEGMENT)

    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    Sid getPropertyStyle(Pid) const override;

public:
    OttavaSegment(Ottava* sp, System* parent);

    OttavaSegment* clone() const override { return new OttavaSegment(*this); }
    Ottava* ottava() const { return (Ottava*)spanner(); }

    EngravingItem* propertyDelegate(Pid) override;

    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    bool canBeExcludedFromOtherParts() const override { return true; }

private:
    void rebaseOffsetsOnAnchorChanged(Grip grip, const PointF& oldPos, System* sys) override;
};

//---------------------------------------------------------
//   @@ Ottava
//   @P ottavaType  enum (Ottava.OTTAVA_8VA, .OTTAVA_8VB, .OTTAVA_15MA, .OTTAVA_15MB, .OTTAVA_22MA, .OTTAVA_22MB)
//---------------------------------------------------------

class Ottava final : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, Ottava)
    DECLARE_CLASSOF(ElementType::OTTAVA)

public:
    Ottava(EngravingItem* parent);
    Ottava(const Ottava&);

    Ottava* clone() const override { return new Ottava(*this); }

    void setOttavaType(OttavaType val);
    OttavaType ottavaType() const { return m_ottavaType; }

    int subtype() const override { return int(ottavaType()); }
    TranslatableString subtypeUserName() const override;

    bool numbersOnly() const { return m_numbersOnly; }
    void setNumbersOnly(bool val);

    void setPlacement(PlacementV);

    LineSegment* createLineSegment(System* parent) override;
    int pitchShift() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleInfo() const override;
    static const char* ottavaTypeName(OttavaType type);

    PointF linePos(Grip grip, System** system) const override;
    bool allowTimeAnchor() const override { return false; }

protected:
    void doComputeEndElement() override;

private:

    friend class OttavaSegment;

    void updateStyledProperties();
    Sid getPropertyStyle(Pid) const override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;

    OttavaType m_ottavaType = OttavaType::OTTAVA_8VA;
    bool m_numbersOnly = false;
};
} // namespace mu::engraving

#endif
