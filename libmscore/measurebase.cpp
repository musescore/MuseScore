//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "measurebase.h"
#include "measure.h"
#include "staff.h"
#include "score.h"
#include "chord.h"
#include "note.h"
#include "layoutbreak.h"
#include "image.h"
#include "segment.h"
#include "tempo.h"
#include "xml.h"
#include "system.h"
#include "stafftypechange.h"

namespace Ms {

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::MeasureBase(Score* score)
   : Element(score)
      {
      setIrregular(true);
      }

MeasureBase::MeasureBase(const MeasureBase& m)
   : Element(m)
      {
      _next     = m._next;
      _prev     = m._prev;
      _tick     = m._tick;
      _no       = m._no;
      _noOffset = m._noOffset;

      for (Element* e : m._el)
            add(e->clone());
      }

//---------------------------------------------------------
//   clearElements
//---------------------------------------------------------

void MeasureBase::clearElements()
      {
      qDeleteAll(_el);
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
      Element::setScore(score);
      for (Element* e : _el)
            e->setScore(score);
      }

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::~MeasureBase()
      {
      qDeleteAll(_el);
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void MeasureBase::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (isMeasure()) {
            for (Element* e : _el) {
                  if (score()->tagIsValid(e->tag())) {
                        if (e->staffIdx() >= score()->staves().size())
                              qDebug("MeasureBase::scanElements: bad staffIdx %d in element %s", e->staffIdx(), e->name());
                        if ((e->track() == -1) || e->systemFlag() || ((Measure*)this)->visible(e->staffIdx()))
                              e->scanElements(data, func, all);
                        }
                  }
            }
      else {
            for (Element* e : _el) {
                  if (score()->tagIsValid(e->tag()))
                        e->scanElements(data, func, all);
                  }
            }
      if (isBox())
            func(data, this);
      }

//---------------------------------------------------------
//   add
///   Add new Element \a el to MeasureBase
//---------------------------------------------------------

void MeasureBase::add(Element* e)
      {
      e->setParent(this);
      if (e->isLayoutBreak()) {
            LayoutBreak* b = toLayoutBreak(e);
            switch (b->layoutBreakType()) {
                  case LayoutBreak::PAGE:
                        setPageBreak(true);
                        setLineBreak(false);
                        setNoBreak(false);
                        break;
                  case LayoutBreak::LINE:
                        setPageBreak(false);
                        setLineBreak(true);
                        setSectionBreak(false);
                        setNoBreak(false);
                        if (b->startWithMeasureOne())
                              triggerLayoutToEnd();
                        break;
                  case LayoutBreak::SECTION:
                        setLineBreak(false);
                        setSectionBreak(true);
                        setNoBreak(false);
                        break;
                  case LayoutBreak::NOBREAK:
                        setPageBreak(false);
                        setLineBreak(false);
                        setSectionBreak(false);
                        setNoBreak(true);
                        break;
                  }
            if (next())
                  next()->triggerLayout();
            }
      triggerLayout();
      _el.push_back(e);
      }

//---------------------------------------------------------
//   remove
///   Remove Element \a el from MeasureBase.
//---------------------------------------------------------

void MeasureBase::remove(Element* el)
      {
      if (el->isLayoutBreak()) {
            LayoutBreak* lb = toLayoutBreak(el);
            switch (lb->layoutBreakType()) {
                  case LayoutBreak::PAGE:
                        setPageBreak(false);
                        break;
                  case LayoutBreak::LINE:
                        setLineBreak(false);
                        break;
                  case LayoutBreak::SECTION:
                        setSectionBreak(false);
                        score()->setPause(endTick(), 0);
                        if (lb->startWithMeasureOne())
                              triggerLayoutToEnd();
                        break;
                  case LayoutBreak::NOBREAK:
                        setNoBreak(false);
                        break;
                  }
            }

      if (!_el.remove(el)) {
            qDebug("MeasureBase(%p)::remove(%s,%p) not found", this, el->name(), el);
            }

      triggerLayout();
      if (next())
            next()->triggerLayout();
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

Measure* MeasureBase::nextMeasure() const
      {
      MeasureBase* m = _next;
      for (;;) {
            if (m == 0 || m->isMeasure())
                  break;
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
      if (mm && score()->styleB(Sid::createMultiMeasureRests) && mm->hasMMRest())
            return mm->mmRest();
      return mm;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

Measure* MeasureBase::prevMeasure() const
      {
      MeasureBase* m = prev();
      while (m) {
            if (m->isMeasure())
                  return toMeasure(m);
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
                              if (mm->hasMMRest())
                                    return mm->mmRest();
                              return mm;
                              }
                        }
                  else
                        return mm;
                  }
            m = m->prev();
            }
      return 0;
      }

//---------------------------------------------------------
//   findPotentialSectionBreak
//---------------------------------------------------------

const MeasureBase *MeasureBase::findPotentialSectionBreak() const
      {
      // we're trying to find the MeasureBase that determines
      // if the next one after this starts a new section
      // if this is a measure, it's the one that determines this
      // but if it is a frame, we may need to look backwards
      const MeasureBase* mb = this;
      while (mb && !mb->isMeasure() && !mb->sectionBreak())
            mb = mb->prev();
      return mb;
      }

//---------------------------------------------------------
//   pause
//---------------------------------------------------------

qreal MeasureBase::pause() const
      {
      return sectionBreak() ? sectionBreakElement()->pause() : 0.0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MeasureBase::layout()
      {
      int breakCount = 0;

      for (Element* element : _el) {
            if (!score()->tagIsValid(element->tag()))
                  continue;
            if (element->isLayoutBreak()) {
                  qreal _spatium = spatium();
                  qreal x;
                  qreal y;
                  if (toLayoutBreak(element)->isNoBreak()) {
                        x = width() - element->width() * .5;
                        y = -(_spatium + element->height());
                        }
                  else {
                        x = -_spatium - element->width() + width()
                            - breakCount * (element->width() + _spatium * .8);
                        y = -2 * _spatium - element->height();
                        breakCount++;
                        }
                  element->setPos(x, y);
                  }
            else if (element->isMarker() || element->isJump())
                  ;
            else
                  element->layout();
            }
      }

//---------------------------------------------------------
//   top
//---------------------------------------------------------

MeasureBase* MeasureBase::top() const
      {
      const MeasureBase* mb = this;
      while (mb->parent()) {
            if (mb->parent()->isMeasureBase())
                  mb = toMeasureBase(mb->parent());
            else
                  break;
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

//---------------------------------------------------------
//   triggerLayout
//---------------------------------------------------------

void MeasureBase::triggerLayout() const
      {
      // for measurebases within other measurebases (e.g., hbox within vbox), use top level
      const MeasureBase* mb = top();
      // avoid triggering layout before getting added to a score
      if (mb->prev() || mb->next())
            score()->setLayout(mb->tick(), -1, mb);
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

QVariant MeasureBase::getProperty(Pid id) const
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
                  return Element::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MeasureBase::setProperty(Pid id, const QVariant& value)
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
                  if (!Element::setProperty(id, value))
                        return false;
                  break;
            }
      triggerLayout();
      score()->setPlaylistDirty();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant MeasureBase::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::REPEAT_END:
            case Pid::REPEAT_START:
            case Pid::REPEAT_JUMP:
                  return false;
            default:
                  break;
            }
      return Element::propertyDefault(propertyId);
      }

//---------------------------------------------------------
//   undoSetBreak
//---------------------------------------------------------

void MeasureBase::undoSetBreak(bool v, LayoutBreak::Type type)
      {
      switch (type) {
            case LayoutBreak::LINE:
                  if (lineBreak() == v)
                        return;
                  setLineBreak(v);
                  break;
            case LayoutBreak::PAGE:
                  if (pageBreak() == v)
                        return;
                  if (v && lineBreak())
                        setLineBreak(false);
                  setPageBreak(v);
                  break;
            case LayoutBreak::SECTION:
                  if (sectionBreak() == v)
                        return;
                  if (v && lineBreak())
                        setLineBreak(false);
                  setSectionBreak(v);
                  break;
            case LayoutBreak::NOBREAK:
                  if (noBreak() == v)
                        return;
                  if (v) {
                        setLineBreak(false);
                        setPageBreak(false);
                        setSectionBreak(false);
                        }
                  setNoBreak(v);
                  break;
            }

      if (v) {
            LayoutBreak* lb = new LayoutBreak(score());
            lb->setLayoutBreakType(type);
            lb->setTrack(-1);       // this are system elements
            MeasureBase* mb = (isMeasure() && toMeasure(this)->isMMRest()) ? toMeasure(this)->mmRestLast() : this;
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
      std::vector<Element*> toDelete;
      for (Element* e : el()) {
            if (e->isLayoutBreak()) {
                  switch (toLayoutBreak(e)->layoutBreakType()) {
                        case LayoutBreak::LINE:
                              if (!lineBreak())
                                    toDelete.push_back(e);
                              break;
                        case LayoutBreak::PAGE:
                              if (!pageBreak())
                                    toDelete.push_back(e);
                              break;
                        case LayoutBreak::SECTION:
                              if (!sectionBreak())
                                    toDelete.push_back(e);
                              break;
                        case LayoutBreak::NOBREAK:
                              if (!noBreak())
                                    toDelete.push_back(e);
                              break;
                        }
                  }
            }
      for (Element* e : toDelete) {
            if (undo)
                  score()->undoRemoveElement(e);
            else
                  _el.remove(e);
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
            return const_cast<Measure*>(toMeasure(_prev)->coveringMMRestOrThis());
            }
      return _prev;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void MeasureBase::writeProperties(XmlWriter& xml) const
      {
      Element::writeProperties(xml);
      for (const Element* e : el())
            e->write(xml);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool MeasureBase::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "LayoutBreak") {
            LayoutBreak* lb = new LayoutBreak(score());
            lb->read(e);
            bool doAdd = true;
            switch (lb->layoutBreakType()) {
                  case LayoutBreak::LINE:
                        if (lineBreak())
                              doAdd = false;
                        break;
                  case LayoutBreak::PAGE:
                        if (pageBreak())
                              doAdd = false;
                        break;
                  case LayoutBreak::SECTION:
                        if (sectionBreak())
                              doAdd = false;
                        break;
                  case LayoutBreak::NOBREAK:
                        if (noBreak())
                              doAdd = false;
                        break;
                  }
            if (doAdd) {
                  add(lb);
                  cleanupLayoutBreaks(false);
                  }
            else
                  delete lb;
            }
      else if (tag == "StaffTypeChange") {
            StaffTypeChange* stc = new StaffTypeChange(score());
            stc->setTrack(e.track());
            stc->setParent(this);
            stc->read(e);
            add(stc);
            }
      else if (Element::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   index
//---------------------------------------------------------

int MeasureBase::index() const
      {
      int idx = 0;
      MeasureBase* m = score()->first();
      while (m) {
            if (m == this)
                  return idx;
            m = m->next();
            ++idx;
            }
      return  -1;
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
            if (m == this)
                  return idx;
            m = m->next();
            if (m && m->isMeasure())
                  ++idx;
            }
      return  -1;
      }

//---------------------------------------------------------
//   sectionBreakElement
//---------------------------------------------------------

LayoutBreak* MeasureBase::sectionBreakElement() const
      {
      if (sectionBreak()) {
            for (Element* e : el()) {
                  if (e->isLayoutBreak() && toLayoutBreak(e)->isSectionBreak())
                        return toLayoutBreak(e);
                  }
            }
      return 0;
      }
}

