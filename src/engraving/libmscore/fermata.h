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

#ifndef __FERMATA_H__
#define __FERMATA_H__

#include "engravingitem.h"

#include "types/types.h"

namespace mu::engraving {
class ChordRest;
class Factory;
class Measure;
class Page;
class Segment;
class System;

//---------------------------------------------------------
//    Fermata
//---------------------------------------------------------

class Fermata final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Fermata)
    DECLARE_CLASSOF(ElementType::FERMATA)

public:

    Fermata(const Fermata&) = default;
    Fermata& operator=(const Fermata&) = delete;

    Fermata* clone() const override { return new Fermata(*this); }

    double mag() const override;

    SymId symId() const { return _symId; }
    void setSymId(SymId id);
    FermataType fermataType() const;
    int subtype() const override;
    TranslatableString typeUserName() const override;

    std::vector<mu::LineF> dragAnchorLines() const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
    void resetProperty(Pid id) override;

    ChordRest* chordRest() const;
    Segment* segment() const { return toSegment(explicitParent()); }
    Measure* measure() const;
    System* system() const;
    Page* page() const;

    double timeStretch() const { return _timeStretch; }
    void setTimeStretch(double val) { _timeStretch = val; }

    bool play() const { return _play; }
    void setPlay(bool val) { _play = val; }

    String accessibleInfo() const override;

protected:
    void added() override;
    void removed() override;

private:
    friend class v0::TLayout;
    friend class Factory;
    Fermata(EngravingItem* parent);

    void draw(mu::draw::Painter*) const override;
    Sid getPropertyStyle(Pid) const override;

    SymId _symId;
    double _timeStretch = -1.0;
    bool _play;
};
} // namespace mu::engraving
#endif
