//=============================================================================
//  MuseScore
//  Music Composition & Notation
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

namespace Ms {

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
            switch (s->segmentType()) {
                  case Segment::Type::Invalid:
                  case Segment::Type::Clef:
                  case Segment::Type::KeySig:
                  case Segment::Type::Ambitus:
                  case Segment::Type::TimeSig:
                  case Segment::Type::StartRepeatBarLine:
                  case Segment::Type::BarLine:
                  case Segment::Type::ChordRest:
                  case Segment::Type::Breath:
                  case Segment::Type::EndBarLine:
                  case Segment::Type::TimeSigAnnounce:
                  case Segment::Type::KeySigAnnounce:
                        break;
                  default:
                        qFatal("SegmentList::check: invalid segment type 0x%x", int(s->segmentType()));
                        break;
                  }
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
            qFatal("SegmentList::check: counted %d but _size is %d", n, _size);
            _size = n;
            }
      }
#endif

//---------------------------------------------------------
//   insert
///   Insert Segment \a e before Segment \a el.
//---------------------------------------------------------

void SegmentList::insert(Segment* e, Segment* el)
      {
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
                  qFatal("SegmentList::insert: already in list");
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
      return first(Segment::Type::ChordRest);
      }

//---------------------------------------------------------
//   first
//---------------------------------------------------------

Segment* SegmentList::first(Segment::Type types) const
      {
      for (Segment* s = _first; s; s = s->next()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

}

