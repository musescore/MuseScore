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

#include "element.h"
#include "layoutbreak.h"

namespace Ms {
class Score;
class System;
class Measure;

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

class MeasureBase : public Element
{
    MeasureBase* _next    { 0 };
    MeasureBase* _prev    { 0 };

    ElementList _el;                      ///< Measure(/tick) relative -elements: with defined start time
                                          ///< but outside the staff
    Fraction _tick         { Fraction(0, 1) };
    int _no                { 0 };         ///< Measure number, counting from zero
    int _noOffset          { 0 };         ///< Offset to measure number
    qreal m_oldWidth       { 0 };         ///< Used to restore layout during recalculations in Score::collectSystem()

protected:
    Fraction _len  { Fraction(0, 1) };    ///< actual length of measure
    void cleanupLayoutBreaks(bool undo);

public:
    MeasureBase(Score* score = 0);
    ~MeasureBase();
    MeasureBase(const MeasureBase&);

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    virtual void setScore(Score* s) override;

    MeasureBase* next() const { return _next; }
    MeasureBase* nextMM() const;
    void setNext(MeasureBase* e) { _next = e; }
    MeasureBase* prev() const { return _prev; }
    MeasureBase* prevMM() const;
    void setPrev(MeasureBase* e) { _prev = e; }
    MeasureBase* top() const;

    Ms::Measure* nextMeasure() const;
    Ms::Measure* prevMeasure() const;
    Ms::Measure* nextMeasureMM() const;
    Ms::Measure* prevMeasureMM() const;

    virtual void write(XmlWriter&) const override = 0;
    virtual void write(XmlWriter&, int, bool, bool) const = 0;

    virtual void layout() override;

    ElementList& el() { return _el; }
    const ElementList& el() const { return _el; }
    System* system() const { return (System*)parent(); }
    void setSystem(System* s) { setParent((Element*)s); }

    const MeasureBase* findPotentialSectionBreak() const;
    LayoutBreak* sectionBreakElement() const;

    void undoSetBreak(bool v, LayoutBreak::Type type);
    void undoSetLineBreak(bool v) { undoSetBreak(v, LayoutBreak::Type::LINE); }
    void undoSetPageBreak(bool v) { undoSetBreak(v, LayoutBreak::Type::PAGE); }
    void undoSetSectionBreak(bool v) { undoSetBreak(v, LayoutBreak::Type::SECTION); }
    void undoSetNoBreak(bool v) { undoSetBreak(v, LayoutBreak::Type::NOBREAK); }

    virtual void moveTicks(const Fraction& diff) { setTick(tick() + diff); }

    virtual void add(Element*) override;
    virtual void remove(Element*) override;
    virtual void writeProperties(XmlWriter&) const override;
    virtual bool readProperties(XmlReader&) override;

    Fraction tick() const override;
    void setTick(const Fraction& f) { _tick = f; }

    Fraction ticks() const { return _len; }
    void setTicks(const Fraction& f) { _len = f; }

    Fraction endTick() const { return _tick + _len; }

    void triggerLayout() const override;

    qreal pause() const;

    virtual QVariant getProperty(Pid) const override;
    virtual bool setProperty(Pid, const QVariant&) override;
    virtual QVariant propertyDefault(Pid) const override;

    void clearElements();
    ElementList takeElements();

    int no() const { return _no; }
    void setNo(int n) { _no = n; }
    int noOffset() const { return _noOffset; }
    void setNoOffset(int n) { _noOffset = n; }

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

    void setOldWidth(qreal n) { m_oldWidth = n; }
    qreal oldWidth() const { return m_oldWidth; }
};
}     // namespace Ms
#endif
