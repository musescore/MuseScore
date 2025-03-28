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

#include "pitchspelling.h"

#include "../types/dimension.h"
#include "../types/types.h"

#include "accidental.h"
#include "types.h"

namespace mu::engraving {
class Factory;

//---------------------------------------------------------
//   @@ Ambitus
//---------------------------------------------------------

class Ambitus final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Ambitus)
    DECLARE_CLASSOF(ElementType::AMBITUS)

public:
    ~Ambitus();

    static constexpr NoteHeadGroup NOTEHEADGROUP_DEFAULT = NoteHeadGroup::HEAD_NORMAL;
    static constexpr NoteHeadType NOTEHEADTYPE_DEFAULT = NoteHeadType::HEAD_AUTO;
    static constexpr DirectionH DIRECTION_DEFAULT = DirectionH::AUTO;
    static constexpr bool HASLINE_DEFAULT = true;
    static const Spatium LINEWIDTH_DEFAULT;
    static constexpr double LINEOFFSET_DEFAULT = 0.8;               // the distance between notehead and line

    Ambitus* clone() const override { return new Ambitus(*this); }

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    double mag() const override;

    void initFrom(Ambitus* a);

    // getters and setters
    NoteHeadGroup noteHeadGroup() const { return m_noteHeadGroup; }
    NoteHeadType noteHeadType() const { return m_noteHeadType; }
    DirectionH direction() const { return m_direction; }
    bool hasLine() const { return m_hasLine; }
    Spatium lineWidth() const { return m_lineWidth; }
    int topOctave() const { return (m_topPitch / 12) - 1; }
    int bottomOctave() const { return (m_bottomPitch / 12) - 1; }
    int topPitch() const { return m_topPitch; }
    int bottomPitch() const { return m_bottomPitch; }
    int topTpc() const { return m_topTpc; }
    int bottomTpc() const { return m_bottomTpc; }

    Accidental* topAccidental() const { return m_topAccidental; }
    Accidental* bottomAccidental() const { return m_bottomAccidental; }

    void setNoteHeadGroup(NoteHeadGroup val) { m_noteHeadGroup = val; }
    void setNoteHeadType(NoteHeadType val) { m_noteHeadType  = val; }
    void setDirection(DirectionH val) { m_direction = val; }
    void setHasLine(bool val) { m_hasLine = val; }
    void setLineWidth(Spatium val) { m_lineWidth = val; }
    void setTopPitch(int val, bool applyLogic = true);
    void setBottomPitch(int val, bool applyLogic = true);
    void setTopTpc(int val, bool applyLogic = true);
    void setBottomTpc(int val, bool applyLogic = true);

    // some utility functions
    Segment* segment() const { return (Segment*)explicitParent(); }
    SymId noteHead() const;
    double headWidth() const;

    // re-implemented virtual functions
    PointF pagePos() const override;        // position in page coordinates
    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    void setTrack(track_idx_t val) override;

    String accessibleInfo() const override;

    void remove(EngravingItem*) override;

    // properties
    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;

    static AccidentalType accidentalType(int tpc, Key key);
    static int staffLine(int tpc, int pitch, ClefType clf);

    struct LayoutData : public EngravingItem::LayoutData {
        PointF topPos;          // position of top note symbol
        PointF bottomPos;       // position of bottom note symbol
        LineF line;             // the drawn line
    };
    DECLARE_LAYOUTDATA_METHODS(Ambitus)

private:

    friend class Factory;
    Ambitus(Segment* parent);
    Ambitus(const Ambitus& a);

    void normalize();

    struct Ranges {
        int topTpc = Tpc::TPC_INVALID;
        int bottomTpc = Tpc::TPC_INVALID;
        int topPitch = INVALID_PITCH;
        int bottomPitch = INVALID_PITCH;
    };

    Ranges estimateRanges() const;                // scan staff up to next section break and update range pitches

    NoteHeadGroup m_noteHeadGroup = NOTEHEADGROUP_DEFAULT;
    NoteHeadType m_noteHeadType = NOTEHEADTYPE_DEFAULT;
    DirectionH m_direction = DIRECTION_DEFAULT;
    bool m_hasLine = HASLINE_DEFAULT;
    Spatium m_lineWidth;
    Accidental* m_topAccidental = nullptr;
    Accidental* m_bottomAccidental = nullptr;
    int m_topPitch = INVALID_PITCH, m_bottomPitch = INVALID_PITCH;
    int m_topTpc = Tpc::TPC_INVALID, m_bottomTpc = Tpc::TPC_INVALID;
};
}
