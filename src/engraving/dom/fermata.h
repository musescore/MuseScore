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

#ifndef MU_ENGRAVING_FERMATA_H
#define MU_ENGRAVING_FERMATA_H

#include "../types/types.h"

#include "engravingitem.h"

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

    SymId symId() const { return m_symId; }
    void setSymId(SymId id) { m_symId = id; }
    void setSymIdAndTimeStretch(SymId id);
    FermataType fermataType() const;
    int subtype() const override;
    TranslatableString subtypeUserName() const override;

    std::vector<LineF> dragAnchorLines() const override;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;
    void resetProperty(Pid id) override;

    ChordRest* chordRest() const;
    Segment* segment() const { return toSegment(explicitParent()); }
    Measure* measure() const;
    System* system() const;
    Page* page() const;

    double timeStretch() const { return m_timeStretch; }
    void setTimeStretch(double val) { m_timeStretch = val; }

    bool play() const { return m_play; }
    void setPlay(bool val) { m_play = val; }

    String accessibleInfo() const override;

protected:
    void added() override;
    void removed() override;

private:

    friend class Factory;
    Fermata(EngravingItem* parent);

    Sid getPropertyStyle(Pid) const override;

    SymId m_symId = SymId::noSym;
    double m_timeStretch = -1.0;
    bool m_play = true;
};
} // namespace mu::engraving
#endif
