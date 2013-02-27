//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "segmentlist.h"
#include "segment.h"
#include "score.h"

//---------------------------------------------------------
//   clone
//---------------------------------------------------------

SegmentList SegmentList::clone() const
      {
      SegmentList dl;
      Segment* s = _first;
      for (int i = 0; i < _size; ++i) {
            Segment* ns = s->clone();
            dl.push_back(ns);
            s = s->next();
            }
      dl.check();
      return dl;
      }

//---------------------------------------------------------
//   check
//---------------------------------------------------------

#ifndef NDEBUG
void SegmentList::check()
      {
      int n = 0;
      Segment* f = 0;
      Segment* l = 0;
      for (Segment* s = _first; s; s = s->next()) {
            if (f == 0)
                  f = s;
            l = s;
            ++n;
            }
      for (Segment* s = _first; s; s = s->next()) {
            Segment* ss = s->next();
            while (ss) {
                  if (s == ss) {
                        qFatal("SegmentList::check: segment twice in list");
                        }
                  ss = ss->next();
                  }
            }
      if (f != _first) {
            qFatal("SegmentList::check: bad first");
            }
      if (l != _last) {
            qFatal("SegmentList::check: bad last");
            }
      if (n != _size) {
            qDebug("SegmentList::check: counted %d but _size is %d", n, _size);
            _size = n;
            abort();
            }
      }
#endif

//---------------------------------------------------------
//   insert
///   Insert Segment \a e before Segment \a el.
//---------------------------------------------------------

void SegmentList::insert(Segment* e, Segment* el)
      {
      if (e->score()->undoRedo())
            qFatal("SegmentList:insert <%s, tick %d> in undo/redo",
               e->subTypeName(), e->tick());
      if (el == 0)
            push_back(e);
      else if (el == first())
            push_front(e);
      else {
            ++_size;
            e->setNext(el);
            e->setPrev(el->prev());
            el->prev()->setNext(e);
            el->setPrev(e);
            check();
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SegmentList::remove(Segment* el)
      {
      if (el->score()->undoRedo())
            qFatal("SegmentList:remove in undo/redo");
      --_size;
      if (el == _first) {
            _first = _first->next();
            if (_first)
                  _first->setPrev(0);
            if (el == _last)
                  _last = 0;
            }
      else if (el == _last) {
            _last = _last->prev();
            if (_last)
                  _last->setNext(0);
            }
      else {
            el->prev()->setNext(el->next());
            el->next()->setPrev(el->prev());
            }
      check();
      }

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void SegmentList::push_back(Segment* e)
      {
      ++_size;
      e->setNext(0);
      if (_last)
            _last->setNext(e);
      else
            _first = e;
      e->setPrev(_last);
      _last = e;
      check();
      }

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void SegmentList::push_front(Segment* e)
      {
      ++_size;
      e->setPrev(0);
      if (_first)
            _first->setPrev(e);
      else
            _last = e;
      e->setNext(_first);
      _first = e;
      check();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void SegmentList::insert(Segment* seg)
      {
#ifndef NDEBUG
//      qDebug("insertSeg <%s> %p %p %p", seg->subTypeName(), seg->prev(), seg, seg->next());
      check();
      for (Segment* s = _first; s; s = s->next()) {
            if (s == seg) {
                  qFatal("SegmentList::insert: already in list\n");
                  }
            }
      if (seg->prev()) {
            Segment* s;
            for (s = _first; s; s = s->next()) {
                  if (s == seg->prev())
                        break;
                  }
            if (s != seg->prev()) {
                  qFatal("SegmentList::insert: seg->prev() not in list");
                  }
            }

      if (seg->next()) {
            Segment* s;
            for (s = _first; s; s = s->next()) {
                  if (s == seg->next())
                        break;
                  }
            if (s != seg->next()) {
                  qFatal("SegmentList::insert: seg->next() not in list");
                  }
            }
#endif
      if (seg->prev())
            seg->prev()->setNext(seg);
      else
            _first = seg;
      if (seg->next())
            seg->next()->setPrev(seg);
      else
            _last = seg;
      ++_size;
      check();
      }

//---------------------------------------------------------
//   firstCRSegment
//---------------------------------------------------------

Segment* SegmentList::firstCRSegment() const
      {
      return first(Segment::SegChordRest);
      }

//---------------------------------------------------------
//   first
//---------------------------------------------------------

Segment* SegmentList::first(Segment::SegmentTypes types) const
      {
      for (Segment* s = _first; s; s = s->next()) {
            if (s->subtype() & types)
                  return s;
            }
      return 0;
      }


