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

#include "engravingitem.h"
#include "durationtype.h"
#include "../types/types.h"

namespace mu::engraving {
//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

/**
 Graphic representation of a shadow note,
 which shows the note insert position in note entry mode.
*/

class ShadowNote final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, ShadowNote)
    DECLARE_CLASSOF(ElementType::SHADOW_NOTE)

public:
    ShadowNote(Score*);

    ShadowNote* clone() const override { return new ShadowNote(*this); }

    bool isValid() const;

    Fraction tick() const override { return m_tick; }
    void setTick(Fraction t) { m_tick = t; }

    int lineIndex() const { return m_lineIndex; }
    void setLineIndex(int n) { m_lineIndex = n; }

    bool isRest() const { return m_isRest; }

    const TDuration& duration() const { return m_duration; }

    void setState(SymId noteSymbol, TDuration duration, bool isRest, double segmentSkylineTopY, double segmentSkylineBottomY,
                  AccidentalType accidentalType = AccidentalType::NONE, const std::set<SymId>& articulationIds = {});

    void drawArticulations(muse::draw::Painter* painter) const;
    void drawMarcato(muse::draw::Painter* painter, const SymId& articulation, RectF& boundRect) const;
    void drawArticulation(muse::draw::Painter* painter, const SymId& articulation, RectF& boundRect) const;

    bool computeUp() const;
    SymId noteheadSymbol() const { return m_noteheadSymbol; }
    bool hasStem() const;
    bool hasFlag() const;
    SymId flagSym() const;
    AccidentalType accidentalType() const;
    const std::set<SymId>& articulationIds() const;

    double segmentSkylineBottomY() const;
    double segmentSkylineTopY() const;

    bool ledgerLinesVisible() const;

private:

    Fraction m_tick;
    int m_lineIndex = -1;
    SymId m_noteheadSymbol = SymId::noSym;
    TDuration m_duration;
    bool m_isRest = false;
    AccidentalType m_accidentalType = AccidentalType::NONE;
    std::set<SymId> m_articulationIds;

    double m_segmentSkylineTopY = 0.0;
    double m_segmentSkylineBottomY = 0.0;
};
} // namespace mu::engraving
