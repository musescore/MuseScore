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

#include "arpeggio.h"
#include "engravingitem.h"

namespace mu::engraving {
class ChordBracket final : public Arpeggio
{
    OBJECT_ALLOCATOR(engraving, ChordBracket)
    DECLARE_CLASSOF(ElementType::CHORD_BRACKET)

public:
    ChordBracket* clone() const override { return new ChordBracket(*this); }

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    void reset() override;

    Spatium hookLength() const { return m_hookLength; }
    DirectionV hookPos() const { return m_hookPos; }
    bool rightSide() const { return m_rightSide; }

private:
    friend class Factory;

    ChordBracket(Chord* parent);

    Spatium m_hookLength = Spatium(1);
    DirectionV m_hookPos = DirectionV::AUTO; // AUTO == both up and down hooks
    bool m_rightSide = false;
};
}
