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

#ifndef __AMBITUS_H__
#define __AMBITUS_H__

#include "engravingitem.h"

#include "pitchspelling.h"

#include "types/types.h"
#include "types/dimension.h"

namespace mu::engraving {
class Accidental;
class Factory;

//---------------------------------------------------------
//   @@ Ambitus
//---------------------------------------------------------

class Ambitus final : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, Ambitus)
    DECLARE_CLASSOF(ElementType::AMBITUS)

    NoteHeadGroup _noteHeadGroup;
    NoteHeadType _noteHeadType;
    DirectionH _dir;
    bool _hasLine;
    Spatium _lineWidth;
    Accidental* _topAccid = nullptr;
    Accidental* _bottomAccid = nullptr;
    int _topPitch, _bottomPitch;
    int _topTpc, _bottomTpc;

    // internally managed, to optimize layout / drawing
    mu::PointF _topPos;       // position of top note symbol
    mu::PointF _bottomPos;    // position of bottom note symbol
    mu::LineF _line;          // the drawn line

    friend class Factory;
    Ambitus(Segment* parent);
    Ambitus(const Ambitus& a);

    void normalize();

public:
    ~Ambitus();

    static constexpr NoteHeadGroup NOTEHEADGROUP_DEFAULT = NoteHeadGroup::HEAD_NORMAL;
    static constexpr NoteHeadType NOTEHEADTYPE_DEFAULT = NoteHeadType::HEAD_AUTO;
    static constexpr DirectionH DIR_DEFAULT = DirectionH::AUTO;
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
    NoteHeadGroup noteHeadGroup() const { return _noteHeadGroup; }
    NoteHeadType noteHeadType() const { return _noteHeadType; }
    DirectionH direction() const { return _dir; }
    bool hasLine() const { return _hasLine; }
    Spatium lineWidth() const { return _lineWidth; }
    int topOctave() const { return (_topPitch / 12) - 1; }
    int bottomOctave() const { return (_bottomPitch / 12) - 1; }
    int topPitch() const { return _topPitch; }
    int bottomPitch() const { return _bottomPitch; }
    int topTpc() const { return _topTpc; }
    int bottomTpc() const { return _bottomTpc; }

    Accidental* topAccidental() const { return _topAccid; }
    Accidental* bottomAccidental() const { return _bottomAccid; }

    void setNoteHeadGroup(NoteHeadGroup val) { _noteHeadGroup = val; }
    void setNoteHeadType(NoteHeadType val) { _noteHeadType  = val; }
    void setDirection(DirectionH val) { _dir = val; }
    void setHasLine(bool val) { _hasLine = val; }
    void setLineWidth(Spatium val) { _lineWidth = val; }
    void setTopPitch(int val, bool applyLogic = true);
    void setBottomPitch(int val, bool applyLogic = true);
    void setTopTpc(int val, bool applyLogic = true);
    void setBottomTpc(int val, bool applyLogic = true);

    // some utility functions
    Segment* segment() const { return (Segment*)explicitParent(); }
    SymId noteHead() const;
    double headWidth() const;

    // re-implemented virtual functions
    void      draw(mu::draw::Painter* painter) const override;
    void      layout() override;
    mu::PointF pagePos() const override;        ///< position in page coordinates
    void      scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;
    void      setTrack(track_idx_t val) override;
    void      write(XmlWriter&) const override;

    String    accessibleInfo() const override;

    void remove(EngravingItem*) override;

    // properties
    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid propertyId, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid id) const override;

    EngravingItem* nextSegmentElement() override;
    EngravingItem* prevSegmentElement() override;

private:

    struct Ranges {
        int topTpc = Tpc::TPC_INVALID;
        int bottomTpc = Tpc::TPC_INVALID;
        int topPitch = INVALID_PITCH;
        int bottomPitch = INVALID_PITCH;
    };

    Ranges estimateRanges() const;                // scan staff up to next section break and update range pitches
};
} // namespace mu::engraving

#endif
