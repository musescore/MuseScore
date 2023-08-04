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

#ifndef __REST_H__
#define __REST_H__

#include "containers.h"

#include "chordrest.h"
#include "notedot.h"

namespace mu::engraving {
class TDuration;

struct RestVerticalClearance {
private:
    int m_above = 0.0; // In space units
    int m_below = 0.0; // In space units
    bool m_locked = false;

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

    ~Rest() { DeleteAll(m_dots); }

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

    SymId getSymbol(DurationType type, int line, int lines);
    void updateSymbol(int line, int lines);

    void checkDots();

    double symWidthNoLedgerLines() const;
    NoteDot* dot(int n);
    const std::vector<NoteDot*>& dotList() const;
    int dotLine() const { return m_dotline; }
    void setDotLine(int l) { m_dotline = l; }

    static int getDotline(DurationType durationType);
    SymId sym() const { return m_sym; }
    void setSym(SymId s) { m_sym = s; }
    bool accent();
    void setAccent(bool flag);

    int computeNaturalLine(int lines); // Natural rest vertical position
    int computeVoiceOffset(int lines); // Vertical displacement in multi-voice cases
    int computeWholeRestOffset(int voiceOffset, int lines);
    bool isWholeRest() const;

    DeadSlapped* deadSlapped() const { return m_deadSlapped; }

    int upLine() const override;
    int downLine() const override;
    mu::PointF stemPos() const override;
    double stemPosX() const override;
    mu::PointF stemPosBeam() const override;
    double rightEdge() const override;

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
    Shape shape() const override;
    void editDrag(EditData& editData) override;

    bool shouldNotBeDrawn() const;

    RestVerticalClearance& verticalClearance() { return m_verticalClearance; }
    const std::vector<Rest*>& mergedRests() const { return m_mergedRests; }

protected:
    Rest(const ElementType& type, Segment* parent = 0);
    Rest(const ElementType& type, Segment* parent, const TDuration&);
    Rest(const Rest&, bool link = false);

    Sid getPropertyStyle(Pid pid) const override;
    virtual mu::RectF numberRect() const { return mu::RectF(); } // TODO: add style to show number over 1-measure rests

private:

    friend class Factory;
    Rest(Segment* parent);
    Rest(Segment* parent, const TDuration&);

    mu::RectF drag(EditData&) override;
    double upPos() const override;
    double downPos() const override;
    void setOffset(const mu::PointF& o) override;

    // values calculated by layout:
    SymId m_sym = SymId::noSym;
    int m_dotline = -1;             // depends on rest symbol
    bool m_gap = false;             // invisible and not selectable for user
    std::vector<NoteDot*> m_dots;
    DeadSlapped* m_deadSlapped = nullptr;

    RestVerticalClearance m_verticalClearance;

    std::vector<Rest*> m_mergedRests; // Rests from other voices that may be merged with this
};
} // namespace mu::engraving
#endif
