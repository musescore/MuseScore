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

#include "slurtie.h"

namespace mu::engraving {
class TieEndPointList;
class TieEndPoint
{
public:
    TieEndPoint(Note* note, String jumpName, bool active, int idx, bool followingNote);
    TieEndPoint() {}

    Note* note() const { return m_note; }
    Tie* endTie() const;
    bool followingNote() const { return m_followingNote; }
    const String& id() const { return m_id; }
    bool active() const { return m_active; }
    void setActive(bool v) { m_active = v; }
    void undoSetActive(bool v);
    void setEndPointList(TieEndPointList* endPointList) { m_endPointList = endPointList; }
    TieEndPointList* endPointList() { return m_endPointList; }

    const String menuTitle() const;

private:
    Note* m_note = nullptr;
    String m_jumpName;
    bool m_active = false;
    String m_id;
    TieEndPointList* m_endPointList = nullptr;
    bool m_followingNote = false;
};

class TieEndPointList
{
public:
    TieEndPointList() = default;
    ~TieEndPointList();

    void add(TieEndPoint* item);
    void clear();
    size_t size() const { return m_endPoints.size(); }
    bool empty() const { return m_endPoints.empty(); }

    void setStartTie(Tie* startTie) { m_startTie = startTie; }
    Tie* startTie() const { return m_startTie; }

    void toggleEndPoint(const String& id);

    void addTie(TieEndPoint* endPoint);
    void removeTie(TieEndPoint* endPoint);

    std::vector<TieEndPoint*>::iterator begin() { return m_endPoints.begin(); }
    std::vector<TieEndPoint*>::const_iterator begin() const { return m_endPoints.begin(); }
    std::vector<TieEndPoint*>::iterator end() { return m_endPoints.end(); }
    std::vector<TieEndPoint*>::const_iterator end() const { return m_endPoints.end(); }

private:
    std::vector<TieEndPoint*> m_endPoints;
    Tie* m_startTie = nullptr;
};

//---------------------------------------------------------
//   @@ TieSegment
///    a single segment of a tie
//---------------------------------------------------------

class TieSegment : public SlurTieSegment
{
    OBJECT_ALLOCATOR(engraving, TieSegment)
    DECLARE_CLASSOF(ElementType::TIE_SEGMENT)

public:
    TieSegment(System* parent);
    TieSegment(const TieSegment& s);

    TieSegment* clone() const override { return new TieSegment(*this); }

    int subtype() const override { return static_cast<int>(spanner()->type()); }

    void addAdjustmentOffset(const PointF& offset, Grip grip) { m_adjustmentOffsets[static_cast<size_t>(grip)] += offset; }
    void resetAdjustmentOffset() { m_adjustmentOffsets.fill(PointF()); }
    PointF adjustmentOffset(Grip grip) { return m_adjustmentOffsets[static_cast<size_t>(grip)]; }
    void consolidateAdjustmentOffsetIntoUserOffset();

    bool isEdited() const;
    void editDrag(EditData&) override;

    Tie* tie() const { return (Tie*)spanner(); }

    void setStaffMove(int val) { m_staffMove = val; }
    staff_idx_t vStaffIdx() const override { return staffIdx() + m_staffMove; }

    virtual double minShoulderHeight() const;
    virtual double maxShoulderHeight() const;
    double endWidth() const override;
    double midWidth() const override;
    double dottedWidth() const override;

    struct LayoutData : public SlurTieSegment::LayoutData {
        bool allEndPointsInactive = false;
    };
    DECLARE_LAYOUTDATA_METHODS(TieSegment)

protected:
    TieSegment(const ElementType& type, System* parent);
    void changeAnchor(EditData&, EngravingItem*) override;

private:
    int m_staffMove = 0;
    std::array<PointF, static_cast<size_t>(Grip::GRIPS)> m_adjustmentOffsets;
};

//---------------------------------------------------------
//   @@ Tie
//!    a Tie has a Note as startElement/endElement
//---------------------------------------------------------

class Tie : public SlurTie
{
    OBJECT_ALLOCATOR(engraving, Tie)
    DECLARE_CLASSOF(ElementType::TIE)

public:
    Tie(EngravingItem* parent = 0);

    Tie* clone() const override { return new Tie(*this); }

    virtual ~Tie() {}

    virtual Note* startNote() const;
    virtual void setStartNote(Note* note);
    virtual Note* endNote() const;
    virtual void setEndNote(Note* note) { setEndElement((EngravingItem*)note); }

    bool isInside() const { return m_isInside; }
    void setIsInside(bool val) { m_isInside = val; }
    virtual bool isOuterTieOfChord(Grip startOrEnd) const;
    bool hasTiedSecondInside() const;
    bool isCrossStaff() const;

    PropertyValue getProperty(Pid propertyId) const override;
    PropertyValue propertyDefault(Pid id) const override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;

    TieSegment* frontSegment() { return toTieSegment(Spanner::frontSegment()); }
    const TieSegment* frontSegment() const { return toTieSegment(Spanner::frontSegment()); }
    TieSegment* backSegment() { return toTieSegment(Spanner::backSegment()); }
    const TieSegment* backSegment() const { return toTieSegment(Spanner::backSegment()); }
    TieSegment* segmentAt(int n) { return toTieSegment(Spanner::segmentAt(n)); }
    const TieSegment* segmentAt(int n) const { return toTieSegment(Spanner::segmentAt(n)); }

    SlurTieSegment* newSlurTieSegment(System* parent) override { return new TieSegment(parent); }

    double scalingFactor() const override;

    // Outgoing ties before repeats
    void collectPossibleEndPoints();
    void addTiesToEndPoints();
    void removeTiesFromEndPoints();
    virtual bool allEndPointsInactive() const;
    virtual TieEndPointList* tieEndPoints();
    virtual const TieEndPointList* tieEndPoints() const;

    // Incoming ties after repeats
    void setEndPoint(TieEndPoint* endPoint) { m_endPoint = endPoint; }
    void updateStartTieOnRemoval();
    TieEndPoint* endPoint() const { return m_endPoint; }
    Tie* startTie() const { return startTieEndPoints() ? startTieEndPoints()->startTie() : nullptr; }

    static Tie* changeTieType(Tie* oldTie, Note* endNote = nullptr);

protected:
    Tie(const ElementType& type, EngravingItem* parent = nullptr);

    bool m_isInside = false;
    M_PROPERTY2(TiePlacement, tiePlacement, setTiePlacement, TiePlacement::AUTO)

    // Endpoint information for incoming ties after repeats
    TieEndPoint* m_endPoint = nullptr;
    TieEndPointList* startTieEndPoints() const { return m_endPoint ? m_endPoint->endPointList() : nullptr; }
};
} // namespace mu::engraving
