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
                  case SegmentType::Invalid:
                  case SegmentType::BeginBarLine:
                  case SegmentType::HeaderClef:
                  case SegmentType::Clef:
                  case SegmentType::KeySig:
                  case SegmentType::Ambitus:
                  case SegmentType::TimeSig:
                  case SegmentType::StartRepeatBarLine:
                  case SegmentType::BarLine:
                  case SegmentType::ChordRest:
                  case SegmentType::Breath:
                  case SegmentType::EndBarLine:
                  case SegmentType::TimeSigAnnounce:
                  case SegmentType::KeySigAnnounce:
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
      if (f && f->prev())
            qFatal("SegmentList::check: first has prev");
      if (l && l->next())
            qFatal("SegmentList::check: last has next");
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
            }
      check();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SegmentList::remove(Segment* e)
      {
#ifndef NDEBUG
      check();
      bool found = false;
      for (Segment* s = _first; s; s = s->next()) {
            if (e == s) {
                  found = true;
                  break;
                  }
            }
      if (!found) {
            qFatal("segment %p %s not in list", e, e->subTypeName());
            }
#endif
      --_size;
      if (e == _first) {
            _first = _first->next();
            if (_first)
                  _first->setPrev(0);
            if (e == _last)
                  _last = 0;
            }
      else if (e == _last) {
            _last = _last->prev();
            if (_last)
                  _last->setNext(0);
            }
      else {
            e->prev()->setNext(e->next());
            e->next()->setPrev(e->prev());
            }
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
//   firstCRSegment
//---------------------------------------------------------

Segment* SegmentList::firstCRSegment() const
      {
      return first(SegmentType::ChordRest);
      }

//---------------------------------------------------------
//   first
//---------------------------------------------------------

Segment* SegmentList::first(SegmentType types) const
      {
      for (Segment* s = _first; s; s = s->next()) {
            if (s->segmentType() & types)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   first
//---------------------------------------------------------

Segment* SegmentList::first(ElementFlag flags) const
      {
      for (Segment* s = _first; s; s = s->next()) {
            if (s->flag(flags))
                  return s;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   last
//---------------------------------------------------------

Segment* SegmentList::last(ElementFlag flags) const
      {
      for (Segment* s = _last; s; s = s->prev()) {
            if (s->flag(flags))
                  return s;
            }
      return nullptr;
      }
}

