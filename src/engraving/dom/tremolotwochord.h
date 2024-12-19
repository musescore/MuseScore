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

#ifndef MU_ENGRAVING_TREMOLOTWOCHORD_H
#define MU_ENGRAVING_TREMOLOTWOCHORD_H

#include "beambase.h"
#include "engravingitem.h"

#include "durationtype.h"
#include "draw/types/painterpath.h"
#include "../types/types.h"
#include "beam.h"
#include "chord.h"

namespace mu::engraving {
class Chord;

class TremoloTwoChord final : public BeamBase
{
    OBJECT_ALLOCATOR(engraving, TremoloTwoChord)
    DECLARE_CLASSOF(ElementType::TREMOLO_TWOCHORD)

public:

    TremoloTwoChord& operator=(const TremoloTwoChord&) = delete;
    TremoloTwoChord* clone() const override { return new TremoloTwoChord(*this); }
    ~TremoloTwoChord() override;

    Chord* chord() const { return toChord(explicitParent()); }

    int subtype() const override { return static_cast<int>(m_tremoloType); }
    TranslatableString subtypeUserName() const override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    void setTremoloType(TremoloType t);
    TremoloType tremoloType() const { return m_tremoloType; }

    double minHeight() const;
    void reset() override;

    PointF chordBeamAnchor(const ChordRest* chord, ChordBeamAnchorType anchorType) const;

    double chordMag() const;
    RectF drag(EditData&) override;

    Chord* chord1() const { return m_chord1; }
    void setChord1(Chord* ch) { m_chord1 = ch; }
    Chord* chord2() const { return m_chord2; }
    void setChord2(Chord* ch) { m_chord2 = ch; }
    void setChords(Chord* c1, Chord* c2)
    {
        m_chord1 = c1;
        m_chord2 = c2;
    }

    PairF beamPos() const;

    TDuration durationType() const;
    void setDurationType(TDuration d);

    Fraction tremoloLen() const;
    int lines() const { return m_lines; }

    bool crossStaffBeamBetween() const;

    PointF pagePos() const override;      ///< position in page coordinates
    String accessibleInfo() const override;
    void triggerLayout() const override;

    TremoloStyle tremoloStyle() const { return m_style; }
    void setTremoloStyle(TremoloStyle v) { m_style = v; }
    void setDirection(DirectionV v) override;
    void setBeamFragment(const BeamFragment& bf) { m_beamFragment = bf; }
    const BeamFragment& beamFragment() const { return m_beamFragment; }
    BeamFragment& beamFragment() { return m_beamFragment; }

    bool playTremolo() const { return m_playTremolo; }
    void setPlayTremolo(bool v) { m_playTremolo = v; }

    bool customStyleApplicable() const;

    PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid propertyId) const override;

    bool needStartEditingAfterSelecting() const override;
    int gripsCount() const override;
    Grip initialEditModeGrip() const override;
    Grip defaultGrip() const override;
    std::vector<PointF> gripsPositions(const EditData&) const override;
    bool isMovable() const override { return true; }
    bool isEditable() const override { return true; }
    void editDrag(EditData&) override;

    void clearBeamSegments() override;

    int maxCRMove() const override;
    int minCRMove() const override;

    //! NOTE for palettes
    muse::draw::PainterPath basePath(double stretch = 0) const;
    const muse::draw::PainterPath& path() const { return m_path; }
    void setPath(const muse::draw::PainterPath& p) { m_path = p; }
    void computeShape();
    //! -----------------

private:
    friend class Factory;

    TremoloTwoChord(Chord* parent);
    TremoloTwoChord(const TremoloTwoChord&);

    void setBeamPos(const PairF& bp);

    TremoloType m_tremoloType = TremoloType::INVALID_TREMOLO;
    Chord* m_chord1 = nullptr;
    Chord* m_chord2 = nullptr;
    TDuration m_durationType;
    bool m_playTremolo = true;

    int m_lines = 0;         // derived from _subtype
    TremoloStyle m_style = TremoloStyle::DEFAULT;
    // for _startAnchor and _slope, once we decide to change trems so that they can
    // continue from one system to the other (or indeed, one measure to the other)
    // we will want a vector of fragments similar to Beam's _beamFragments structure.
    // for now, a single fragment is sufficient
    BeamFragment m_beamFragment;

    //! NOTE for palette
    PainterPath m_path;
};
} // namespace mu::engraving
#endif
