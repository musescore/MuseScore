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

#ifndef __MEASUREBASE_H__
#define __MEASUREBASE_H__

/**
 \file
 Definition of MeasureBase class.
*/

#include "engravingitem.h"

namespace mu::engraving {
class LayoutBreak;
class Measure;
class Score;
class System;

//---------------------------------------------------------
//   Repeat
//---------------------------------------------------------

enum class Repeat : char {
    NONE    = 0,
    END     = 1,
    START   = 2,
    MEASURE = 4,
    JUMP    = 8
};

constexpr Repeat operator|(Repeat t1, Repeat t2)
{
    return static_cast<Repeat>(static_cast<int>(t1) | static_cast<int>(t2));
}

constexpr bool operator&(Repeat t1, Repeat t2)
{
    return static_cast<int>(t1) & static_cast<int>(t2);
}

//---------------------------------------------------------
//   @@ MeasureBase
///    Virtual base class for Measure, HBox and VBox
//
//   @P lineBreak       bool        true if a system break is positioned on this measure
//   @P nextMeasure     Measure     the next Measure (read-only)
//   @P nextMeasureMM   Measure     the next multi-measure rest Measure (read-only)
//   @P pageBreak       bool        true if a page break is positioned on this measure
//   @P prevMeasure     Measure     the previous Measure (read-only)
//   @P prevMeasureMM   Measure     the previous multi-measure rest Measure (read-only)
//---------------------------------------------------------

class MeasureBase : public EngravingItem
{
    OBJECT_ALLOCATOR(engraving, MeasureBase)

public:

    ~MeasureBase();

    System* system() const { return (System*)explicitParent(); }
    void setParent(System* s) { EngravingItem::setParent((EngravingObject*)(s)); }

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObjectList scanChildren() const override;
    virtual void scanElements(void* data, void (* func)(void*, EngravingItem*), bool all=true) override;

    virtual void setScore(Score* s) override;

    MeasureBase* next() const { return m_next; }
    MeasureBase* nextMM() const;
    void setNext(MeasureBase* e) { m_next = e; }
    MeasureBase* prev() const { return m_prev; }
    MeasureBase* prevMM() const;
    void setPrev(MeasureBase* e) { m_prev = e; }
    MeasureBase* top() const;

    Measure* nextMeasure() const;
    Measure* prevMeasure() const;
    Measure* nextMeasureMM() const;
    Measure* prevMeasureMM() const;

    ElementList& el() { return m_el; }
    const ElementList& el() const { return m_el; }

    const MeasureBase* findPotentialSectionBreak() const;
    LayoutBreak* sectionBreakElement() const;

    void undoSetBreak(bool v, LayoutBreakType type);
    void undoSetLineBreak(bool v) { undoSetBreak(v, LayoutBreakType::LINE); }
    void undoSetPageBreak(bool v) { undoSetBreak(v, LayoutBreakType::PAGE); }
    void undoSetSectionBreak(bool v) { undoSetBreak(v, LayoutBreakType::SECTION); }
    void undoSetNoBreak(bool v) { undoSetBreak(v, LayoutBreakType::NOBREAK); }

    void cleanupLayoutBreaks(bool undo);

    virtual void moveTicks(const Fraction& diff) { setTick(tick() + diff); }

    virtual void add(EngravingItem*) override;
    virtual void remove(EngravingItem*) override;

    Fraction tick() const override;
    void setTick(const Fraction& f);

    Fraction ticks() const { return m_len; }
    void setTicks(const Fraction& f) { m_len = f; }

    Fraction endTick() const { return m_tick + m_len; }

    void triggerLayout() const override;

    double pause() const;

    PropertyValue getProperty(Pid) const override;
    bool setProperty(Pid, const PropertyValue&) override;
    PropertyValue propertyDefault(Pid) const override;

    void clearElements();
    ElementList takeElements();

    int no() const { return m_no; }
    void setNo(int n) { m_no = n; }
    int noOffset() const { return m_noOffset; }
    void setNoOffset(int n) { m_noOffset = n; }

    bool repeatEnd() const { return flag(ElementFlag::REPEAT_END); }
    void setRepeatEnd(bool v) { setFlag(ElementFlag::REPEAT_END, v); }

    bool repeatStart() const { return flag(ElementFlag::REPEAT_START); }
    void setRepeatStart(bool v) { setFlag(ElementFlag::REPEAT_START, v); }

    bool repeatJump() const { return flag(ElementFlag::REPEAT_JUMP); }
    void setRepeatJump(bool v) { setFlag(ElementFlag::REPEAT_JUMP, v); }

    bool irregular() const { return flag(ElementFlag::IRREGULAR); }
    void setIrregular(bool v) { setFlag(ElementFlag::IRREGULAR, v); }

    bool lineBreak() const { return flag(ElementFlag::LINE_BREAK); }
    void setLineBreak(bool v) { setFlag(ElementFlag::LINE_BREAK, v); }

    bool pageBreak() const { return flag(ElementFlag::PAGE_BREAK); }
    void setPageBreak(bool v) { setFlag(ElementFlag::PAGE_BREAK, v); }

    bool sectionBreak() const { return flag(ElementFlag::SECTION_BREAK); }
    void setSectionBreak(bool v) { setFlag(ElementFlag::SECTION_BREAK, v); }

    bool noBreak() const { return flag(ElementFlag::NO_BREAK); }
    void setNoBreak(bool v) { setFlag(ElementFlag::NO_BREAK, v); }

    bool hasCourtesyKeySig() const { return flag(ElementFlag::KEYSIG); }
    void setHasCourtesyKeySig(int v) { setFlag(ElementFlag::KEYSIG, v); }

    virtual void computeMinWidth() { }

    int index() const;
    int measureIndex() const;

    void setOldWidth(double n) { m_oldWidth = n; }
    double oldWidth() const { return m_oldWidth; }

protected:

    MeasureBase(const ElementType& type, System* system = 0);
    MeasureBase(const MeasureBase&);

    Fraction m_len  { Fraction(0, 1) };    // actual length of measure

private:
    MeasureBase* m_next = nullptr;
    MeasureBase* m_prev = nullptr;

    ElementList m_el;                     // Measure(/tick) relative -elements: with defined start time
                                          // but outside the staff
    Fraction m_tick         { Fraction(0, 1) };
    int m_no = 0;                         // Measure number, counting from zero
    int m_noOffset = 0;                   // Offset to measure number
    double m_oldWidth = 0.0;              // Used to restore layout during recalculations in Score::collectSystem()
};

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

class MeasureBaseList
{
public:
    MeasureBaseList();
    MeasureBase* first() const { return m_first; }
    MeasureBase* last()  const { return m_last; }
    void clear() { m_first = m_last = 0; m_size = 0; }
    void add(MeasureBase*);
    void remove(MeasureBase*);
    void insert(MeasureBase*, MeasureBase*);
    void remove(MeasureBase*, MeasureBase*);
    void change(MeasureBase* o, MeasureBase* n);
    int size() const { return m_size; }
    bool empty() const { return m_size == 0; }

private:
    void push_back(MeasureBase* e);
    void push_front(MeasureBase* e);

    int m_size = 0;
    MeasureBase* m_first = nullptr;
    MeasureBase* m_last = nullptr;
};
} // namespace mu::engraving
#endif
