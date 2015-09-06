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
#include "lyrics.h"
#include "score.h"
#include "chord.h"
#include "note.h"
#include "layoutbreak.h"
#include "image.h"
#include "segment.h"
#include "tempo.h"

namespace Ms {

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::MeasureBase(Score* score)
   : Element(score)
      {
      _prev = 0;
      _next = 0;
      _breakHint    = false;
      _lineBreak    = false;
      _pageBreak    = false;
      _sectionBreak = 0;
      }

MeasureBase::MeasureBase(const MeasureBase& m)
   : Element(m)
      {
      _next         = m._next;
      _prev         = m._prev;
      _tick         = m._tick;
      _breakHint    = m._breakHint;
      _lineBreak    = m._lineBreak;
      _pageBreak    = m._pageBreak;
      _sectionBreak = m._sectionBreak ? new LayoutBreak(*m._sectionBreak) : 0;

      foreach(Element* e, m._el)
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
      if (_sectionBreak)
            _sectionBreak->setScore(score);
      foreach (Element* e, _el)
            e->setScore(score);
      }

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::~MeasureBase()
      {
      foreach(Element* e, _el)
            delete e;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void MeasureBase::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      if (isMeasure()) {
            foreach(Element* e, _el) {
                  if (score()->tagIsValid(e->tag())) {
                        if (e->staffIdx() >= score()->staves().size())
                              qDebug("MeasureBase::scanElements: bad staffIdx %d in element %s", e->staffIdx(), e->name());
                        if ((e->track() == -1) || e->systemFlag() || ((Measure*)this)->visible(e->staffIdx()))
                              e->scanElements(data, func, all);
                        }
                  }
            }
      else {
            foreach(Element* e, _el) {
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
      if (e->type() == Element::Type::LAYOUT_BREAK) {
            LayoutBreak* b = static_cast<LayoutBreak*>(e);
            foreach (Element* ee, _el) {
                  if (ee->type() == Element::Type::LAYOUT_BREAK && static_cast<LayoutBreak*>(ee)->layoutBreakType() == b->layoutBreakType()) {
                        if (MScore::debugMode)
                              qDebug("warning: layout break already set");
                        return;
                        }
                  }
            switch (b->layoutBreakType()) {
                  case LayoutBreak::Type::PAGE:
                        _pageBreak = true;
                        break;
                  case LayoutBreak::Type::LINE:
                        _lineBreak = true;
                        break;
                  case LayoutBreak::Type::SECTION:
                        _sectionBreak = b;
//does not work with repeats: score()->tempomap()->setPause(endTick(), b->pause());
                        score()->setLayoutAll(true);
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
      if (el->type() == Element::Type::LAYOUT_BREAK) {
            LayoutBreak* lb = static_cast<LayoutBreak*>(el);
            switch (lb->layoutBreakType()) {
                  case LayoutBreak::Type::PAGE:
                        _pageBreak = false;
                        break;
                  case LayoutBreak::Type::LINE:
                        _lineBreak = false;
                        break;
                  case LayoutBreak::Type::SECTION:
                        _sectionBreak = 0;
                        score()->setPause(endTick(), 0);
                        score()->addLayoutFlags(LayoutFlag::FIX_TICKS);
                        score()->setLayoutAll(true);
                        break;
                  }
            }
      if (!_el.remove(el)) {
            qDebug("MeasureBase(%p)::remove(%s,%p) not found", this, el->name(), el);
            //abort();
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
      return _sectionBreak ? _sectionBreak->pause() : 0.0;
      }

//---------------------------------------------------------
//   isStartOfSection
//    returns true if start of score.
//    returns true if start of a new section.
//          (caveat: section breaks may occur on measure base objects that aren't actual measures.)
//---------------------------------------------------------

bool MeasureBase::isStartOfSection() const
      {
      MeasureBase* m = _prev;
      while (m) {
            // if first reach a section break, then is start of a new section
            if (m->sectionBreak())
                  return true;

            // if encounter an actual measure before encountering a section break, then is not start of a section
            if (m->isMeasure())
                  return false;

            m = m->prev();
            }

      return true;      // if reach start of score, then is start of a section
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void MeasureBase::layout()
      {
      int breakCount = 0;

      foreach (Element* element, _el) {
            if (!score()->tagIsValid(element->tag()))
                  continue;
            if (element->type() == Element::Type::LAYOUT_BREAK) {
                  qreal _spatium = spatium();
                  qreal x = -_spatium - element->width() + width()
                            - breakCount * (element->width() + _spatium * .8);
                  qreal y = -2 * _spatium - element->height();
                  element->setPos(x, y);
                  breakCount++;
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
      switch(id) {
            case P_ID::BREAK_HINT:
                  return QVariant(_breakHint);
            default:
                  return Element::getProperty(id);
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool MeasureBase::setProperty(P_ID id, const QVariant& property)
      {
      switch(id) {
            case P_ID::BREAK_HINT:
                  _breakHint = property.toBool();
                  break;
            default:
                  if (!Element::setProperty(id, property))
                        return false;
                  break;
            }
      return true;
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void MeasureBase::undoSetBreak(bool v, LayoutBreak::Type type)
      {
      if (v) {
            LayoutBreak* lb = new LayoutBreak(_score);
            lb->setLayoutBreakType(type);
            lb->setTrack(-1);       // this are system elements
            lb->setParent(this);
            _score->undoAddElement(lb);
            }
      else {
            // remove line break
            foreach(Element* e, el()) {
                  if (e->type() == Element::Type::LAYOUT_BREAK && static_cast<LayoutBreak*>(e)->layoutBreakType() ==type) {
                        _score->undoRemoveElement(e);
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   nextMM
//---------------------------------------------------------

MeasureBase* MeasureBase::nextMM() const
      {
      if (_next
         && _next->type() == Element::Type::MEASURE
         && score()->styleB(StyleIdx::createMultiMeasureRests)
         && static_cast<Measure*>(_next)->hasMMRest()) {
            return static_cast<Measure*>(_next)->mmRest();
            }
      return _next;
      }
}

