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

#ifndef __OTTAVA_H__
#define __OTTAVA_H__

#include "textlinebase.h"
#include "property.h"

namespace mu::engraving {
//---------------------------------------------------------
//   OttavaE
//---------------------------------------------------------

struct OttavaE {
    int offset;
    unsigned start;
    unsigned end;
};

//---------------------------------------------------------
//   OttavaType
//---------------------------------------------------------

enum class OttavaType : char {
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
    OttavaType type;
    int shift;
    const char* name;
};

// order is important, should be the same as OttavaType
static const OttavaDefault ottavaDefault[] = {
    { OttavaType::OTTAVA_8VA,  12,  "8va" },
    { OttavaType::OTTAVA_8VB,  -12, "8vb" },
    { OttavaType::OTTAVA_15MA, 24,  "15ma" },
    { OttavaType::OTTAVA_15MB, -24, "15mb" },
    { OttavaType::OTTAVA_22MA, 36,  "22ma" },
    { OttavaType::OTTAVA_22MB, -36, "22mb" }
};

class Ottava;

//---------------------------------------------------------
//   @@ OttavaSegment
//---------------------------------------------------------

class OttavaSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, OttavaSegment)

    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;
    Sid getPropertyStyle(Pid) const override;

public:
    OttavaSegment(Ottava* sp, System* parent);

    OttavaSegment* clone() const override { return new OttavaSegment(*this); }
    Ottava* ottava() const { return (Ottava*)spanner(); }
    void layout() override;
    EngravingItem* propertyDelegate(Pid) override;
};

//---------------------------------------------------------
//   @@ Ottava
//   @P ottavaType  enum (Ottava.OTTAVA_8VA, .OTTAVA_8VB, .OTTAVA_15MA, .OTTAVA_15MB, .OTTAVA_22MA, .OTTAVA_22MB)
//---------------------------------------------------------

class Ottava final : public TextLineBase
{
    OBJECT_ALLOCATOR(engraving, Ottava)
    DECLARE_CLASSOF(ElementType::OTTAVA)

    OttavaType _ottavaType;
    bool _numbersOnly;

    void updateStyledProperties();
    Sid getPropertyStyle(Pid) const override;
    void undoChangeProperty(Pid id, const PropertyValue&, PropertyFlags ps) override;

protected:
    friend class OttavaSegment;

public:
    Ottava(EngravingItem* parent);
    Ottava(const Ottava&);

    Ottava* clone() const override { return new Ottava(*this); }

    void setOttavaType(OttavaType val);
    OttavaType ottavaType() const { return _ottavaType; }

    bool numbersOnly() const { return _numbersOnly; }
    void setNumbersOnly(bool val);

    void setPlacement(PlacementV);

    LineSegment* createLineSegment(System* parent) override;
    int pitchShift() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    String accessibleInfo() const override;
    static const char* ottavaTypeName(OttavaType type);
};
} // namespace mu::engraving

#endif
