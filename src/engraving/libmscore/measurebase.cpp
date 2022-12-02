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

#include "rw/xml.h"

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

namespace mu::engraving {
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
    _next     = m._next;
    _prev     = m._prev;
    _tick     = m._tick;
    _no       = m._no;
    _noOffset = m._noOffset;

    for (EngravingItem* e : m._el) {
        add(e->clone());
    }
}

//---------------------------------------------------------
//   clearElements
//---------------------------------------------------------

void MeasureBase::clearElements()
{
    DeleteAll(_el);
    _el.clear();
}

//---------------------------------------------------------
//   takeElements
//---------------------------------------------------------

ElementList MeasureBase::takeElements()
{
    ElementList l = _el;
    _el.clear();
    return l;
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void MeasureBase::setScore(Score* score)
{
    EngravingItem::setScore(score);
    for (EngravingItem* e : _el) {
        e->setScore(score);
    }
}

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::~MeasureBase()
{
    DeleteAll(_el);
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
    _el.push_back(e);
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
    if (!_el.remove(el)) {
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
    MeasureBase* m = _next;
    for (;;) {
        if (m == 0 || m->isMeasure()) {
            break;
        }
        m = m->_next;
    }
    return toMeasure(m);
}

//---------------------------------------------------------
//   nextMeasureMM
//---------------------------------------------------------

Measure* MeasureBase::nextMeasureMM() const
{
    Measure* mm = nextMeasure();
    if (mm && score()->styleB(Sid::createMultiMeasureRests) && mm->hasMMRest()) {
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
            if (score()->styleB(Sid::createMultiMeasureRests)) {
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
//   layout
//---------------------------------------------------------

void MeasureBase::layout()
{
    int breakCount = 0;

    for (EngravingItem* element : _el) {
        if (!score()->tagIsValid(element->tag())) {
            continue;
        }
        if (element->isLayoutBreak()) {
            double _spatium = spatium();
            double x;
            double y;
            if (toLayoutBreak(element)->isNoBreak()) {
                x = width() + score()->styleMM(Sid::barWidth) - element->width() * .5;
            } else {
                x = width() + score()->styleMM(Sid::barWidth) - element->width()
                    - breakCount * (element->width() + _spatium * .5);
                breakCount++;
            }
            y = -2.5 * _spatium - element->height();
            element->setPos(x, y);
        } else if (element->isMarker() || element->isJump()) {
        } else {
            element->layout();
        }
    }
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
    return mb ? mb->_tick : Fraction(-1, 1);
}

void MeasureBase::setTick(const Fraction& f)
{
    _tick = f;
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
        for (EngravingItem* e : _el) {
            if (score()->tagIsValid(e->tag())) {
                staff_idx_t staffIdx = e->staffIdx();
                if (staffIdx != mu::nidx && staffIdx >= score()->staves().size()) {
                    LOGD("MeasureBase::scanElements: bad staffIdx %zu in element %s", staffIdx, e->typeName());
                }
                if ((e->track() == mu::nidx) || e->systemFlag() || toMeasure(this)->visible(staffIdx)) {
                    e->scanElements(data, func, all);
                }
            }
        }
    } else {
        for (EngravingItem* e : _el) {
            if (score()->tagIsValid(e->tag())) {
                e->scanElements(data, func, all);
            }
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
    return _measures.first();
}

//---------------------------------------------------------
//   last
//---------------------------------------------------------

MeasureBase* Score::last()  const
{
    return _measures.last();
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
            _el.remove(e);
        }
    }
}

//---------------------------------------------------------
//   nextMM
//---------------------------------------------------------

MeasureBase* MeasureBase::nextMM() const
{
    if (_next
        && _next->isMeasure()
        && score()->styleB(Sid::createMultiMeasureRests)
        && toMeasure(_next)->hasMMRest()) {
        return toMeasure(_next)->mmRest();
    }
    return _next;
}

//---------------------------------------------------------
//   prevMM
//---------------------------------------------------------

MeasureBase* MeasureBase::prevMM() const
{
    if (_prev
        && _prev->isMeasure()
        && score()->styleB(Sid::createMultiMeasureRests)) {
        return const_cast<Measure*>(toMeasure(_prev)->mmRest1());
    }
    return _prev;
}

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void MeasureBase::writeProperties(XmlWriter& xml) const
{
    EngravingItem::writeProperties(xml);
    for (const EngravingItem* e : el()) {
        e->write(xml);
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool MeasureBase::readProperties(XmlReader& e)
{
    const AsciiStringView tag(e.name());
    if (tag == "LayoutBreak") {
        LayoutBreak* lb = Factory::createLayoutBreak(this);
        lb->read(e);
        bool doAdd = true;
        switch (lb->layoutBreakType()) {
        case LayoutBreakType::LINE:
            if (lineBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::PAGE:
            if (pageBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::SECTION:
            if (sectionBreak()) {
                doAdd = false;
            }
            break;
        case LayoutBreakType::NOBREAK:
            if (noBreak()) {
                doAdd = false;
            }
            break;
        }
        if (doAdd) {
            add(lb);
            cleanupLayoutBreaks(false);
        } else {
            delete lb;
        }
    } else if (tag == "StaffTypeChange") {
        StaffTypeChange* stc = Factory::createStaffTypeChange(this);
        stc->setTrack(e.context()->track());
        stc->setParent(this);
        stc->read(e);
        add(stc);
    } else if (EngravingItem::readProperties(e)) {
    } else {
        return false;
    }
    return true;
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
    while (m) {
        if (m == this) {
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
}
