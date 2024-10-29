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

#ifndef MU_ENGRAVING_REST_H
#define MU_ENGRAVING_REST_H

#include "containers.h"

#include "chordrest.h"
#include "notedot.h"

namespace mu::engraving {
class TDuration;

struct RestVerticalClearance {
public:
    void reset()
    {
        m_above = 1000; // arbitrary high value
        m_below = 1000; // arbitrary high value
        m_locked = false;
    }

    int above() const { return m_above; }
    int below() const { return m_below; }
    void setAbove(int v) { m_above = std::max(std::min(m_above, v), 0); }
    void setBelow(int v) { m_below = std::max(std::min(m_below, v), 0); }

    bool locked() const { return m_locked; }
    void setLocked(bool v) { m_locked = v; }

private:
    int m_above = 0.0; // In space units
    int m_below = 0.0; // In space units
    bool m_locked = false;
};

//---------------------------------------------------------
//    @@ Rest
///     This class implements a rest.
//---------------------------------------------------------

class Rest : public ChordRest
{
    OBJECT_ALLOCATOR(engraving, Rest)
    DECLARE_CLASSOF(ElementType::REST)

public:

    ~Rest() { muse::DeleteAll(m_dots); }

    void hack_toRestType();

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;

    Rest& operator=(const Rest&) = delete;

    Rest* clone() const override { return new Rest(*this, false); }
    EngravingItem* linkedClone() override { return new Rest(*this, true); }
    Measure* measure() const override { return explicitParent() ? toMeasure(explicitParent()->explicitParent()) : 0; }
    double mag() const override;
    double intrinsicMag() const override;

    void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all = true) override;
    void setTrack(track_idx_t val) override;

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;

    bool isGap() const { return m_gap; }
    virtual void setGap(bool v) { m_gap = v; }

    virtual void add(EngravingItem*) override;
    virtual void remove(EngravingItem*) override;

    void checkDots();

    NoteDot* dot(int n);
    const std::vector<NoteDot*>& dotList() const;
    int dotLine() const { return m_dotline; }
    void setDotLine(int l) { m_dotline = l; }

    static int getDotline(DurationType durationType);
    bool accent();
    void setAccent(bool flag);

    bool isWholeRest() const;
    bool isBreveRest() const;

    DeadSlapped* deadSlapped() const { return m_deadSlapped; }

    int upLine() const override;
    int downLine() const override;
    PointF stemPos() const override;
    double stemPosX() const override;
    PointF stemPosBeam() const override;
    double rightEdge() const override;
    double centerX() const;

    void localSpatiumChanged(double oldValue, double newValue) override;
    PropertyValue propertyDefault(Pid) const override;
    void resetProperty(Pid id) override;
    bool setProperty(Pid propertyId, const PropertyValue& v) override;
    PropertyValue getProperty(Pid propertyId) const override;
    void undoChangeDotsVisible(bool v);

    EngravingItem* nextElement() override;
    EngravingItem* prevElement() override;
    String accessibleInfo() const override;
    String screenReaderInfo() const override;
    void editDrag(EditData& editData) override;

    bool shouldNotBeDrawn() const;

    RestVerticalClearance& verticalClearance() { return m_verticalClearance; }

    struct LayoutData : public ChordRest::LayoutData {
        std::vector<Rest*> mergedRests;     // Rests from other voices that may be merged with this
        ld_field<SymId> sym = { "[Rest] sym", SymId::restQuarter };
    };
    DECLARE_LAYOUTDATA_METHODS(Rest)

    int computeNaturalLine(int lines) const; // Natural rest vertical position
    int computeVoiceOffset(int lines, LayoutData* ldata) const; // Vertical displacement in multi-voice cases
    int computeWholeOrBreveRestOffset(int voiceOffset, int lines) const;

    SymId getSymbol(DurationType type, int line, int lines) const;
    void updateSymbol(int line, int lines, LayoutData* ldata) const;
    double symWidthNoLedgerLines(LayoutData* ldata) const;

protected:
    Rest(const ElementType& type, Segment* parent = 0);
    Rest(const ElementType& type, Segment* parent, const TDuration&);
    Rest(const Rest&, bool link = false);

    Sid getPropertyStyle(Pid pid) const override;
    virtual RectF numberRect() const { return RectF(); } // TODO: add style to show number over 1-measure rests

private:

    friend class Factory;
    Rest(Segment* parent);
    Rest(Segment* parent, const TDuration&);

    RectF drag(EditData&) override;
    double upPos() const override;
    double downPos() const override;
    void setOffset(const PointF& o) override;

    // values calculated by layout:

    int m_dotline = -1;             // depends on rest symbol
    bool m_gap = false;             // invisible and not selectable for user
    std::vector<NoteDot*> m_dots;
    DeadSlapped* m_deadSlapped = nullptr;

    RestVerticalClearance m_verticalClearance;
};
} // namespace mu::engraving
#endif
