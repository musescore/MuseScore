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
    muse::DeleteAll(m_el);
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
    muse::DeleteAll(m_el);
}

System* MeasureBase::prevNonVBoxSystem() const
{
    bool mmRests = score()->style().styleB(Sid::createMultiMeasureRests);
    System* curSystem = system();
    IF_ASSERT_FAILED(curSystem) {
        return nullptr;
    }

    System* prevSystem = curSystem;
    for (const MeasureBase* mb = this; mb && prevSystem == curSystem; mb = mmRests ? mb->prevMM() : mb->prev()) {
        if (mb->isMeasure() || mb->isHBox()) {
            prevSystem = mb->system();
        } else {
            return nullptr;
        }
    }

    return prevSystem != curSystem ? prevSystem : nullptr;
}

System* MeasureBase::nextNonVBoxSystem() const
{
    bool mmRests = score()->style().styleB(Sid::createMultiMeasureRests);
    System* curSystem = system();
    IF_ASSERT_FAILED(curSystem) {
        return nullptr;
    }

    System* nextSystem = curSystem;
    for (const MeasureBase* mb = this; mb && nextSystem == curSystem; mb = mmRests ? mb->nextMM() : mb->next()) {
        if (mb->isMeasure() || mb->isHBox()) {
            nextSystem = mb->system();
        } else {
            return nullptr;
        }
    }

    return nextSystem != curSystem ? nextSystem : nullptr;
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
            if (b->startWithMeasureOne()) {
                triggerLayoutToEnd();
            }
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
            if (lb->startWithMeasureOne()) {
                triggerLayoutToEnd();
            }
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

    triggerLayout();
    if (next()) {
        next()->triggerLayout();
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
    return nullptr;
}

//---------------------------------------------------------
//   prevMeasureMM
//---------------------------------------------------------

Measure* MeasureBase::prevMeasureMM() const
{
    Measure* m = prevMeasure();
    if (m) {
        return m->coveringMMRestOrThis();
    }

    return nullptr;
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
//   getInScore
//---------------------------------------------------------

MeasureBase* MeasureBase::getInScore(Score* score, bool useNextMeasureFallback) const
{
    MeasureBase* newMB = nullptr;
    if (!isMeasure() && !excludeFromOtherParts()) {
        for (auto e : linkList()) {
            if (e->score() == score) {
                newMB = toMeasureBase(e);
                break;
            }
        }
    }
    if (isMeasure() || (!newMB && useNextMeasureFallback)) {
        newMB = score->tick2measure(tick());
    }
    if (!newMB) {
        LOGD("measure base not found in score");
    }
    return newMB;
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
        score()->setLayout(mb->tick(), muse::nidx, mb);
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
            if (staffIdx != muse::nidx && staffIdx >= score()->staves().size()) {
                LOGD("MeasureBase::scanElements: bad staffIdx %zu in element %s", staffIdx, e->typeName());
            }
            if ((e->track() == muse::nidx) || e->systemFlag() || toMeasure(this)->visible(staffIdx)) {
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
    if (id == Pid::IRREGULAR || id == Pid::NO_OFFSET) {
        triggerLayoutAll();
    } else {
        triggerLayout();
    }
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
        lb->setTrack(0);
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

bool MeasureBase::isBefore(const EngravingItem* other) const
{
    if (other->isMeasureBase()) {
        const MeasureBase* otherMb = toMeasureBase(other);
        return isBefore(otherMb);
    }

    return EngravingItem::isBefore(other);
}

bool MeasureBase::isBefore(const MeasureBase* other) const
{
    if (this == other) {
        return false;
    }

    Fraction otherTick = other->tick();
    if (otherTick != m_tick) {
        return m_tick < otherTick;
    }

    if (this->isMeasure() && other->isMeasure()) {
        // (this == other) has already been excluded, so this is only
        // possible if one is the overlying mmRest starting on the other.
        // Let's set by convention that the mmRest isBefore the underlying measure.
        return toMeasure(this)->isMMRest();
    }

    bool otherIsMMRest = other->isMeasure() && toMeasure(other)->isMMRest();
    for (const MeasureBase* mb = otherIsMMRest ? nextMM() : next(); mb && mb->tick() == m_tick;
         mb = otherIsMMRest ? mb->nextMM() : mb->next()) {
        if (mb == other) {
            return true;
        }
    }

    return false;
}

const SystemLock* MeasureBase::systemLock() const
{
    return score()->systemLocks()->lockContaining(this);
}

bool MeasureBase::isStartOfSystemLock() const
{
    const SystemLock* lock = score()->systemLocks()->lockStartingAt(this);
    return lock != nullptr;
}

bool MeasureBase::isEndOfSystemLock() const
{
    const SystemLock* lock = systemLock();
    return lock && lock->endMB() == this;
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
    m_first = nullptr;
    m_last  = nullptr;
    m_size  = 0;
}

void MeasureBaseList::clear()
{
    m_first = nullptr;
    m_last = nullptr;
    m_size = 0;
    m_tickIndex.clear();
}

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void MeasureBaseList::push_back(MeasureBase* m)
{
    ++m_size;
    if (m_last) {
        m_last->setNext(m);
        m->setPrev(m_last);
        m->setNext(0);
    } else {
        m_first = m;
        m->setPrev(0);
        m->setNext(0);
    }
    m_last = m;
}

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void MeasureBaseList::push_front(MeasureBase* m)
{
    ++m_size;
    if (m_first) {
        m_first->setPrev(m);
        m->setNext(m_first);
        m->setPrev(0);
    } else {
        m_last = m;
        m->setPrev(0);
        m->setNext(0);
    }
    m_first = m;
}

//---------------------------------------------------------
//   add
//    insert m before m->next()
//---------------------------------------------------------

void MeasureBaseList::add(MeasureBase* m)
{
    MeasureBase* el = m->next();
    if (el == 0) {
        push_back(m);
        return;
    }
    if (el == m_first) {
        push_front(m);
        return;
    }
    ++m_size;
    m->setPrev(el->prev());
    el->prev()->setNext(m);
    el->setPrev(m);
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MeasureBaseList::remove(MeasureBase* m)
{
    --m_size;
    if (m->prev()) {
        m->prev()->setNext(m->next());
    } else {
        m_first = m->next();
    }
    if (m->next()) {
        m->next()->setPrev(m->prev());
    } else {
        m_last = m->prev();
    }
}

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void MeasureBaseList::insert(MeasureBase* fm, MeasureBase* lm)
{
    for (MeasureBase* m = fm; m != lm; m = m->next()) {
        ++m_size;
    }
    ++m_size;
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
    for (MeasureBase* m = fm; m != lm; m = m->next()) {
        --m_size;
    }
    --m_size;
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

//---------------------------------------------------------
//   append
//    append measure to the end of the list and update
//    tick index
//---------------------------------------------------------

void MeasureBaseList::append(MeasureBase* m)
{
    assert(!m->next());
    assert(!m->prev() || m->prev() == m_last);

    ++m_size;
    if (m_last) {
        m_last->setNext(m);
        m->setPrev(m_last);
        m->setNext(0);
    } else {
        m_first = m;
        m->setPrev(0);
        m->setNext(0);
    }
    m_last = m;
    m_tickIndex.emplace(std::make_pair(m->tick().ticks(), m));
}

Measure* MeasureBaseList::measureByTick(int tick) const
{
    if (empty() || tick > m_last->endTick().ticks()) {
        return nullptr;
    }

    auto it = m_tickIndex.upper_bound(tick);

    if (it == m_tickIndex.begin()) {
        MeasureBase* mb = it->second;

        if (mb->isMeasure()) {
            return toMeasure(mb);
        }
        return nullptr;
    }

    --it;
    for (;; --it) {
        if (it == m_tickIndex.begin()) {
            MeasureBase* mb = it->second;

            if (mb->isMeasure()) {
                return toMeasure(mb);
            }
            return nullptr;
        }

        MeasureBase* mb = it->second;
        if (!mb) {
            break;
        }

        if (mb->isMeasure()) {
            return toMeasure(mb);
        }
    }

    return nullptr;
}

std::vector<MeasureBase*> MeasureBaseList::measureBasesAtTick(int tick) const
{
    std::vector<MeasureBase*> result;
    if (empty() || tick > m_last->endTick().ticks()) {
        return result;
    }

    result = muse::values(m_tickIndex, tick);

    return result;
}

void MeasureBaseList::updateTickIndex()
{
    m_tickIndex.clear();

    for (MeasureBase* mb = m_first; mb; mb = mb->next()) {
        m_tickIndex.emplace(std::make_pair(mb->tick().ticks(), mb));
    }
}
