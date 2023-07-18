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

#include "measurebase.h"

#include "factory.h"
#include "layoutbreak.h"
#include "measure.h"
#include "score.h"
#include "staff.h"
#include "stafftypechange.h"
#include "system.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::MeasureBase(const ElementType& type, System* system)
    : EngravingItem(type, system)
{
    setIrregular(true);
}

MeasureBase::MeasureBase(const MeasureBase& m)
    : EngravingItem(m)
{
    m_next     = m.m_next;
    m_prev     = m.m_prev;
    m_tick     = m.m_tick;
    m_no       = m.m_no;
    m_noOffset = m.m_noOffset;

    for (EngravingItem* e : m.m_el) {
        add(e->clone());
    }
}

//---------------------------------------------------------
//   clearElements
//---------------------------------------------------------

void MeasureBase::clearElements()
{
    DeleteAll(m_el);
    m_el.clear();
}

//---------------------------------------------------------
//   takeElements
//---------------------------------------------------------

ElementList MeasureBase::takeElements()
{
    ElementList l = m_el;
    m_el.clear();
    return l;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void MeasureBase::setScore(Score* score)
{
    EngravingItem::setScore(score);
    for (EngravingItem* e : m_el) {
        e->setScore(score);
    }
}

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::~MeasureBase()
{
    DeleteAll(m_el);
}

//---------------------------------------------------------
//   add
///   Add new EngravingItem \a el to MeasureBase
//---------------------------------------------------------

void MeasureBase::add(EngravingItem* e)
{
    if (e->explicitParent() != this) {
        e->setParent(this);
    }

    if (e->isLayoutBreak()) {
        LayoutBreak* b = toLayoutBreak(e);
        switch (b->layoutBreakType()) {
        case LayoutBreakType::PAGE:
            setPageBreak(true);
            setLineBreak(false);
            setNoBreak(false);
            break;
        case LayoutBreakType::LINE:
            setPageBreak(false);
            setLineBreak(true);
            setSectionBreak(false);
            setNoBreak(false);
            break;
        case LayoutBreakType::SECTION:
            setLineBreak(false);
            setSectionBreak(true);
            setNoBreak(false);
            //does not work with repeats: score()->tempomap()->setPause(endTick(), b->pause());
            triggerLayoutAll();
            break;
        case LayoutBreakType:: NOBREAK:
            setPageBreak(false);
            setLineBreak(false);
            setSectionBreak(false);
            setNoBreak(true);
            break;
        }
        if (next()) {
            next()->triggerLayout();
        }
//            triggerLayoutAll();     // TODO
    }
    triggerLayout();
    m_el.push_back(e);
    e->added();
}

//---------------------------------------------------------
//   remove
///   Remove EngravingItem \a el from MeasureBase.
//---------------------------------------------------------

void MeasureBase::remove(EngravingItem* el)
{
    if (el->isLayoutBreak()) {
        LayoutBreak* lb = toLayoutBreak(el);
        switch (lb->layoutBreakType()) {
        case LayoutBreakType::PAGE:
            setPageBreak(false);
            break;
        case LayoutBreakType::LINE:
            setLineBreak(false);
            break;
        case LayoutBreakType::SECTION:
            setSectionBreak(false);
            score()->setPause(endTick(), 0);
            triggerLayoutAll();
            break;
        case LayoutBreakType::NOBREAK:
            setNoBreak(false);
            break;
        }
    }
    if (!m_el.remove(el)) {
        LOGD("MeasureBase(%p)::remove(%s,%p) not found", this, el->typeName(), el);
    } else {
        el->removed();
    }
}

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

Measure* MeasureBase::nextMeasure() const
{
    MeasureBase* m = m_next;
    for (;;) {
        if (m == 0 || m->isMeasure()) {
            break;
        }
        m = m->m_next;
    }
    return toMeasure(m);
}

//---------------------------------------------------------
//   nextMeasureMM
//---------------------------------------------------------

Measure* MeasureBase::nextMeasureMM() const
{
    Measure* mm = nextMeasure();
    if (mm && style().styleB(Sid::createMultiMeasureRests) && mm->hasMMRest()) {
        return mm->mmRest();
    }
    return mm;
}

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

Measure* MeasureBase::prevMeasure() const
{
    MeasureBase* m = prev();
    while (m) {
        if (m->isMeasure()) {
            return toMeasure(m);
        }
        m = m->prev();
    }
    return 0;
}

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

Measure* MeasureBase::prevMeasureMM() const
{
    MeasureBase* m = prev();
    while (m) {
        if (m->isMeasure()) {
            Measure* mm = toMeasure(m);
            if (style().styleB(Sid::createMultiMeasureRests)) {
                if (mm->mmRestCount() >= 0) {
                    if (mm->hasMMRest()) {
                        return mm->mmRest();
                    }
                    return mm;
                }
            } else {
                return mm;
            }
        }
        m = m->prev();
    }
    return 0;
}

//---------------------------------------------------------
//   findPotentialSectionBreak
//---------------------------------------------------------

const MeasureBase* MeasureBase::findPotentialSectionBreak() const
{
    // we're trying to find the MeasureBase that determines
    // if the next one after this starts a new section
    // if this is a measure, it's the one that determines this
    // but if it is a frame, we may need to look backwards
    const MeasureBase* mb = this;
    while (mb && !mb->isMeasure() && !mb->sectionBreak()) {
        mb = mb->prev();
    }
    return mb;
}

//---------------------------------------------------------
//   pause
//---------------------------------------------------------

double MeasureBase::pause() const
{
    const LayoutBreak* layoutBreak = sectionBreakElement();
    return layoutBreak ? layoutBreak->pause() : 0.0;
}

//---------------------------------------------------------
//   top
//---------------------------------------------------------

MeasureBase* MeasureBase::top() const
{
    const MeasureBase* mb = this;
    while (mb->explicitParent()) {
        if (mb->explicitParent()->isMeasureBase()) {
            mb = toMeasureBase(mb->explicitParent());
        } else {
            break;
        }
    }
    return const_cast<MeasureBase*>(mb);
}

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

Fraction MeasureBase::tick() const
{
    const MeasureBase* mb = top();
    return mb ? mb->m_tick : Fraction(-1, 1);
}

void MeasureBase::setTick(const Fraction& f)
{
    m_tick = f;
}

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void MeasureBase::triggerLayout() const
{
    // for measurebases within other measurebases (e.g., hbox within vbox), use top level
    const MeasureBase* mb = top();
    // avoid triggering layout before getting added to a score
    if (mb->prev() || mb->next()) {
        score()->setLayout(mb->tick(), mu::nidx, mb);
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void MeasureBase::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    if (isMeasure()) {
        for (EngravingItem* e : m_el) {
            staff_idx_t staffIdx = e->staffIdx();
            if (staffIdx != mu::nidx && staffIdx >= score()->staves().size()) {
                LOGD("MeasureBase::scanElements: bad staffIdx %zu in element %s", staffIdx, e->typeName());
            }
            if ((e->track() == mu::nidx) || e->systemFlag() || toMeasure(this)->visible(staffIdx)) {
                e->scanElements(data, func, all);
            }
        }
    } else {
        for (EngravingItem* e : m_el) {
            e->scanElements(data, func, all);
        }
    }
    if (isBox()) {
        func(data, this);
    }
}

//---------------------------------------------------------
//   first
//---------------------------------------------------------

MeasureBase* Score::first() const
{
    return m_measures.first();
}

//---------------------------------------------------------
//   last
//---------------------------------------------------------

MeasureBase* Score::last()  const
{
    return m_measures.last();
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

PropertyValue MeasureBase::getProperty(Pid id) const
{
    switch (id) {
    case Pid::REPEAT_END:
        return repeatEnd();
    case Pid::REPEAT_START:
        return repeatStart();
    case Pid::REPEAT_JUMP:
        return repeatJump();
    case Pid::NO_OFFSET:
        return noOffset();
    case Pid::IRREGULAR:
        return irregular();
    default:
        return EngravingItem::getProperty(id);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MeasureBase::setProperty(Pid id, const PropertyValue& value)
{
    switch (id) {
    case Pid::REPEAT_END:
        setRepeatEnd(value.toBool());
        break;
    case Pid::REPEAT_START:
        setRepeatStart(value.toBool());
        break;
    case Pid::REPEAT_JUMP:
        setRepeatJump(value.toBool());
        break;
    case Pid::NO_OFFSET:
        setNoOffset(value.toInt());
        break;
    case Pid::IRREGULAR:
        setIrregular(value.toBool());
        break;
    default:
        if (!EngravingItem::setProperty(id, value)) {
            return false;
        }
        break;
    }
    triggerLayoutAll();
    score()->setPlaylistDirty();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

PropertyValue MeasureBase::propertyDefault(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::REPEAT_END:
    case Pid::REPEAT_START:
    case Pid::REPEAT_JUMP:
        return false;
    default:
        break;
    }
    return EngravingItem::propertyDefault(propertyId);
}

//---------------------------------------------------------
//   undoSetBreak
//---------------------------------------------------------

void MeasureBase::undoSetBreak(bool v, LayoutBreakType type)
{
    switch (type) {
    case LayoutBreakType::LINE:
        if (lineBreak() == v) {
            return;
        }
        setLineBreak(v);
        break;
    case LayoutBreakType::PAGE:
        if (pageBreak() == v) {
            return;
        }
        if (v && lineBreak()) {
            setLineBreak(false);
        }
        setPageBreak(v);
        break;
    case LayoutBreakType::SECTION:
        if (sectionBreak() == v) {
            return;
        }
        if (v && lineBreak()) {
            setLineBreak(false);
        }
        setSectionBreak(v);
        break;
    case LayoutBreakType::NOBREAK:
        if (noBreak() == v) {
            return;
        }
        if (v) {
            setLineBreak(false);
            setPageBreak(false);
            setSectionBreak(false);
        }
        setNoBreak(v);
        break;
    }

    if (v) {
        MeasureBase* mb = (isMeasure() && toMeasure(this)->isMMRest()) ? toMeasure(this)->mmRestLast() : this;
        LayoutBreak* lb = Factory::createLayoutBreak(mb);
        lb->setLayoutBreakType(type);
        lb->setTrack(mu::nidx);           // this are system elements
        lb->setParent(mb);
        score()->undoAddElement(lb);
    }
    cleanupLayoutBreaks(true);
}

//---------------------------------------------------------
//   cleanupLayoutBreaks
//---------------------------------------------------------

void MeasureBase::cleanupLayoutBreaks(bool undo)
{
    // remove unneeded layout breaks
    std::vector<EngravingItem*> toDelete;
    for (EngravingItem* e : el()) {
        if (e->isLayoutBreak()) {
            switch (toLayoutBreak(e)->layoutBreakType()) {
            case LayoutBreakType::LINE:
                if (!lineBreak()) {
                    toDelete.push_back(e);
                }
                break;
            case LayoutBreakType::PAGE:
                if (!pageBreak()) {
                    toDelete.push_back(e);
                }
                break;
            case LayoutBreakType::SECTION:
                if (!sectionBreak()) {
                    toDelete.push_back(e);
                }
                break;
            case LayoutBreakType::NOBREAK:
                if (!noBreak()) {
                    toDelete.push_back(e);
                }
                break;
            }
        }
    }
    for (EngravingItem* e : toDelete) {
        if (undo) {
            score()->undoRemoveElement(e);
        } else {
            m_el.remove(e);
        }
    }
}

//---------------------------------------------------------
//   nextMM
//---------------------------------------------------------

MeasureBase* MeasureBase::nextMM() const
{
    if (m_next
        && m_next->isMeasure()
        && style().styleB(Sid::createMultiMeasureRests)
        && toMeasure(m_next)->hasMMRest()) {
        return toMeasure(m_next)->mmRest();
    }
    return m_next;
}

//---------------------------------------------------------
//   prevMM
//---------------------------------------------------------

MeasureBase* MeasureBase::prevMM() const
{
    if (m_prev
        && m_prev->isMeasure()
        && style().styleB(Sid::createMultiMeasureRests)) {
        return const_cast<Measure*>(toMeasure(m_prev)->coveringMMRestOrThis());
    }
    return m_prev;
}

//---------------------------------------------------------
//   index
//---------------------------------------------------------

int MeasureBase::index() const
{
    int idx = 0;
    MeasureBase* m = score()->first();
    Measure* mmRestFirst = nullptr;
    if (isMeasure() && toMeasure(this)->isMMRest()) {
        mmRestFirst = toMeasure(this)->mmRestFirst();
    }
    while (m) {
        if (m == this || m == mmRestFirst) {
            return idx;
        }
        m = m->next();
        ++idx;
    }
    return -1;
}

//---------------------------------------------------------
//   measureIndex
//    returns index of measure counting only Measures but
//    skipping other MeasureBase descendants
//---------------------------------------------------------

int MeasureBase::measureIndex() const
{
    int idx = 0;
    MeasureBase* m = score()->firstMeasure();
    Measure* mmRestFirst = nullptr;
    if (isMeasure() && toMeasure(this)->isMMRest()) {
        mmRestFirst = toMeasure(this)->mmRestFirst();
    }
    while (m) {
        if (m == this || m == mmRestFirst) {
            return idx;
        }
        m = m->next();
        if (m && m->isMeasure()) {
            ++idx;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   sectionBreakElement
//---------------------------------------------------------

LayoutBreak* MeasureBase::sectionBreakElement() const
{
    if (sectionBreak()) {
        for (EngravingItem* e : el()) {
            if (e->isLayoutBreak() && toLayoutBreak(e)->isSectionBreak()) {
                return toLayoutBreak(e);
            }
        }
    }

    return nullptr;
}

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

MeasureBaseList::MeasureBaseList()
{
    m_first = 0;
    m_last  = 0;
    m_size  = 0;
}

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void MeasureBaseList::push_back(MeasureBase* e)
{
    ++m_size;
    if (m_last) {
        m_last->setNext(e);
        e->setPrev(m_last);
        e->setNext(0);
    } else {
        m_first = e;
        e->setPrev(0);
        e->setNext(0);
    }
    m_last = e;
}

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void MeasureBaseList::push_front(MeasureBase* e)
{
    ++m_size;
    if (m_first) {
        m_first->setPrev(e);
        e->setNext(m_first);
        e->setPrev(0);
    } else {
        m_last = e;
        e->setPrev(0);
        e->setNext(0);
    }
    m_first = e;
}

//---------------------------------------------------------
//   add
//    insert e before e->next()
//---------------------------------------------------------

void MeasureBaseList::add(MeasureBase* e)
{
    MeasureBase* el = e->next();
    if (el == 0) {
        push_back(e);
        return;
    }
    if (el == m_first) {
        push_front(e);
        return;
    }
    ++m_size;
    e->setPrev(el->prev());
    el->prev()->setNext(e);
    el->setPrev(e);
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* el)
{
    --m_size;
    if (el->prev()) {
        el->prev()->setNext(el->next());
    } else {
        m_first = el->next();
    }
    if (el->next()) {
        el->next()->setPrev(el->prev());
    } else {
        m_last = el->prev();
    }
}

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void MeasureBaseList::insert(MeasureBase* fm, MeasureBase* lm)
{
    ++m_size;
    for (MeasureBase* m = fm; m != lm; m = m->next()) {
        ++m_size;
    }
    MeasureBase* pm = fm->prev();
    if (pm) {
        pm->setNext(fm);
    } else {
        m_first = fm;
    }
    MeasureBase* nm = lm->next();
    if (nm) {
        nm->setPrev(lm);
    } else {
        m_last = lm;
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* fm, MeasureBase* lm)
{
    --m_size;
    for (MeasureBase* m = fm; m != lm; m = m->next()) {
        --m_size;
    }
    MeasureBase* pm = fm->prev();
    MeasureBase* nm = lm->next();
    if (pm) {
        pm->setNext(nm);
    } else {
        m_first = nm;
    }
    if (nm) {
        nm->setPrev(pm);
    } else {
        m_last = pm;
    }
}

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void MeasureBaseList::change(MeasureBase* ob, MeasureBase* nb)
{
    nb->setPrev(ob->prev());
    nb->setNext(ob->next());
    if (ob->prev()) {
        ob->prev()->setNext(nb);
    }
    if (ob->next()) {
        ob->next()->setPrev(nb);
    }
    if (ob == m_last) {
        m_last = nb;
    }
    if (ob == m_first) {
        m_first = nb;
    }
    if (nb->type() == ElementType::HBOX || nb->type() == ElementType::VBOX
        || nb->type() == ElementType::TBOX || nb->type() == ElementType::FBOX) {
        nb->setParent(ob->system());
    }
    for (EngravingItem* e : nb->el()) {
        e->setParent(nb);
    }
}
