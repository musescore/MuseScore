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

#ifndef __RASGUEADO_H__
#define __RASGUEADO_H__

#include "chordtextlinebase.h"

namespace mu::engraving {
class Rasgueado;

//---------------------------------------------------------
//   @@ RasgueadoSegment
//---------------------------------------------------------

class RasgueadoSegment final : public TextLineBaseSegment
{
    OBJECT_ALLOCATOR(engraving, RasgueadoSegment)
public:
    RasgueadoSegment(Rasgueado* sp, System* parent);

    RasgueadoSegment* clone() const override { return new RasgueadoSegment(*this); }

    Rasgueado* rasgueado() const { return (Rasgueado*)spanner(); }

    void layout() override;

    friend class Rasgueado;
};

//---------------------------------------------------------
//   @@ Rasgueado
//---------------------------------------------------------

class Rasgueado final : public ChordTextLineBase
{
    OBJECT_ALLOCATOR(engraving, Rasgueado)
    DECLARE_CLASSOF(ElementType::RASGUEADO)

public:
    Rasgueado(EngravingItem* parent);

    Rasgueado* clone() const override { return new Rasgueado(*this); }

    LineSegment* createLineSegment(System* parent) override;

    PropertyValue propertyDefault(Pid propertyId) const override;
    Sid getPropertyStyle(Pid) const override;
};
} // namespace mu::engraving
#endif
