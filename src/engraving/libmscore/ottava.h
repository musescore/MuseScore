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

namespace Ms {
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
    void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;
    Sid getPropertyStyle(Pid) const override;

public:
    OttavaSegment(Spanner* sp, Score* s)
        : TextLineBaseSegment(sp, s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF) { }

    ElementType type() const override { return ElementType::OTTAVA_SEGMENT; }
    OttavaSegment* clone() const override { return new OttavaSegment(*this); }
    Ottava* ottava() const { return (Ottava*)spanner(); }
    void layout() override;
    Element* propertyDelegate(Pid) override;
};

//---------------------------------------------------------
//   @@ Ottava
//   @P ottavaType  enum (Ottava.OTTAVA_8VA, .OTTAVA_8VB, .OTTAVA_15MA, .OTTAVA_15MB, .OTTAVA_22MA, .OTTAVA_22MB)
//---------------------------------------------------------

class Ottava final : public TextLineBase
{
    OttavaType _ottavaType;
    bool _numbersOnly;

    void updateStyledProperties();
    Sid getPropertyStyle(Pid) const override;
    void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;

protected:
    friend class OttavaSegment;

public:
    Ottava(Score* s);
    Ottava(const Ottava&);

    Ottava* clone() const override { return new Ottava(*this); }
    ElementType type() const override { return ElementType::OTTAVA; }

    void setOttavaType(OttavaType val);
    OttavaType ottavaType() const { return _ottavaType; }

    bool numbersOnly() const { return _numbersOnly; }
    void setNumbersOnly(bool val);

    void setPlacement(Placement);

    LineSegment* createLineSegment() override;
    int pitchShift() const;

    void write(XmlWriter& xml) const override;
    void read(XmlReader& de) override;
    bool readProperties(XmlReader& e) override;

    QVariant getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const QVariant&) override;
    QVariant propertyDefault(Pid) const override;
    Pid propertyId(const QStringRef& xmlName) const override;

    QString accessibleInfo() const override;
    static const char* ottavaTypeName(OttavaType type);
};
}     // namespace Ms

#endif
