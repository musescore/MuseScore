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

namespace Ms {

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::MeasureBase(Score* score)
   : Element(score)
      {
      }

MeasureBase::MeasureBase(const MeasureBase& m)
   : Element(m)
      {
      _next          = m._next;
      _prev          = m._prev;
      _tick          = m._tick;
      _lineBreak     = m._lineBreak;
      _pageBreak     = m._pageBreak;
      _sectionBreak  = m._sectionBreak;
      _no            = m._no;
      _noOffset      = m._noOffset;
      _irregular     = m._irregular;
      _repeatEnd     = m._repeatEnd;
      _repeatStart   = m._repeatStart;
      _repeatJump    = m._repeatJump;

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
                        _pageBreak    = true;
                        _lineBreak    = false;
                        _noBreak      = false;
                        break;
                  case LayoutBreak::LINE:
                        _pageBreak    = false;
                        _lineBreak    = true;
                        _sectionBreak = false;
                        _noBreak      = false;
                        break;
                  case LayoutBreak::SECTION:
                        _lineBreak    = false;
                        _sectionBreak = true;
                        _noBreak      = false;
      //does not work with repeats: score()->tempomap()->setPause(endTick(), b->pause());
                        score()->setLayoutAll();
                        break;
                  case LayoutBreak::NOBREAK:
                        _pageBreak    = false;
                        _lineBreak    = false;
                        _sectionBreak = false;
                        _noBreak      = true;
                        break;
                  }
            }
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
                        _pageBreak = false;
                        break;
                  case LayoutBreak::LINE:
                        _lineBreak = false;
                        break;
                  case LayoutBreak::SECTION:
                        _sectionBreak = false;
                        score()->setPause(endTick(), 0);
                        score()->setLayoutAll();
                        break;
                  case LayoutBreak::NOBREAK:
                        _noBreak = false;
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
            if (m == 0 || m->type() == Element::Type::MEASURE)
                  break;
            m = m->_next;
            }
      return static_cast<Measure*>(m);
      }

//---------------------------------------------------------
//   nextMeasureMM
//---------------------------------------------------------

Measure* MeasureBase::nextMeasureMM() const
      {
      Measure* mm = nextMeasure();
      if (mm && score()->styleB(StyleIdx::createMultiMeasureRests) && mm->hasMMRest())
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
            if (m->type() == Element::Type::MEASURE)
                  return static_cast<Measure*>(m);
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
            if (m->type() == Element::Type::MEASURE) {
                  Measure* mm = static_cast<Measure*>(m);
                  if (score()->styleB(StyleIdx::createMultiMeasureRests)) {
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
      return _sectionBreak ? sectionBreakElement()->pause() : 0.0;
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

QVariant MeasureBase::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::REPEAT_END:
                  return repeatEnd();
            case P_ID::REPEAT_START:
                  return repeatStart();
            case P_ID::REPEAT_JUMP:
                  return repeatJump();
            default:
                  return Element::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MeasureBase::setProperty(P_ID id, const QVariant& value)
      {
      switch (id) {
            case P_ID::REPEAT_END:
                  setRepeatEnd(value.toBool());
                  break;
            case P_ID::REPEAT_START:
                  setRepeatStart(value.toBool());
                  break;
            case P_ID::REPEAT_JUMP:
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

QVariant MeasureBase::propertyDefault(P_ID propertyId) const
      {
      switch (propertyId) {
            case P_ID::REPEAT_END:
            case P_ID::REPEAT_START:
            case P_ID::REPEAT_JUMP:
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
                  break;
            case LayoutBreak::PAGE:
                  if (pageBreak() == v)
                        return;
                  if (v && lineBreak())
                        setLineBreak(false);
                  break;
            case LayoutBreak::SECTION:
                  if (sectionBreak() == v)
                        return;
                  if (v && lineBreak())
                        setLineBreak(false);
                  break;
            case LayoutBreak::NOBREAK:
                  if (noBreak() == v)
                        return;
                  if (v) {
                        setLineBreak(false);
                        setPageBreak(false);
                        setSectionBreak(false);
                        }
                  break;
            }

      if (v) {
            LayoutBreak* lb = new LayoutBreak(score());
            lb->setLayoutBreakType(type);
            lb->setTrack(-1);       // this are system elements
            lb->setParent(this);
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
         && score()->styleB(StyleIdx::createMultiMeasureRests)
         && static_cast<Measure*>(_next)->hasMMRest()) {
            return toMeasure(_next)->mmRest();
            }
      return _next;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void MeasureBase::writeProperties(Xml& xml) const
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
      else if (Element::readProperties(e))
            ;
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   index
//    for debugging only
//---------------------------------------------------------

int MeasureBase::index() const
      {
      int idx = 0;
      MeasureBase* m = score()->first();
      while (m) {
            if (m == this)
                  return idx;
            m = m->next();
            }
      return  -1;
      }

//---------------------------------------------------------
//   sectionBreakElement
//---------------------------------------------------------

LayoutBreak* MeasureBase::sectionBreakElement() const
      {
      if (_sectionBreak) {
            for (Element* e : el()) {
                  if (e->isLayoutBreak() && toLayoutBreak(e)->isSectionBreak())
                        return toLayoutBreak(e);
                  }
            }
      return 0;
      }
}

