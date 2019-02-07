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
      // if we are to empty the elementList, we need to make
      // sure that the measure-flags are updated
      setSectionBreak(false); // only SET if an LayoutBreak::SECTION element is present.

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
                        break;
                  case LayoutBreak::SECTION:
                        setLineBreak(false);
                        setSectionBreak(true);
                        setNoBreak(false);
      //does not work with repeats: score()->tempomap()->setPause(endTick(), b->pause());
                        score()->setLayoutAll();
                        break;
                  case LayoutBreak::NOBREAK:
                        setPageBreak(false);
                        setLineBreak(false);
                        setSectionBreak(false);
                        setNoBreak(true);
                        break;
                  }
            if (next())
                  score()->setLayout(next()->endTick());
//            score()->setLayoutAll();     // TODO
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
                        score()->setLayoutAll();
                        break;
                  case LayoutBreak::NOBREAK:
                        setNoBreak(false);
                        break;
                  }
            }
      if (!_el.remove(el)) {
            qDebug("MeasureBase(%p)::remove(%s,%p) not found", this, el->name(), el);
            }
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
            default:
                  if (!Element::setProperty(id, value))
                        return false;
                  break;
            }
      score()->setLayoutAll();
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
// This function names hints of a possible KLUDGE somewhere else.
// The score is obvious in an inconsistent state, and a
// stop-gap-measure function like this is needed.
//---------------------------------------------------------

void MeasureBase::cleanupLayoutBreaks(bool undo)
      {
      // remove unneeded layout breaks
      std::vector<Element*> toDelete;
      bool has_lb_section_elm = false;
      // Remove layout-break-elements when no such ElementFlag
      // is set on the measure.
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
                              has_lb_section_elm = true;
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

      // Reset the ElementFlag::SECTION_BREAK when no
      // associated layout-break elements are found.
      if(sectionBreak() && (! has_lb_section_elm)){
            // Emergency correct the measure flag, this is needed
            // because alot of code relies on the fact that
            // if sectionBreak() holds, then sectionBreakElement() must return an element.
            setSectionBreak(false);
            qWarning("measure %d has sectionBreak flag, but no element of type ElementType::LAYOUT_BREAK\n", no());
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
//   afrac
//---------------------------------------------------------

Fraction MeasureBase::afrac() const
      {
      return Fraction::fromTicks(_tick);
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
      MeasureBase* m = score()->first();
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
            /* If we end up here, then we have a measure
             * where the sectionBreak flag is set, but no
             * section-break-element is found in the measure's
             * element-list.
             */
            qWarning("Measure %d, without a section-break-element", no());
            }
      return nullptr;
      }
}
