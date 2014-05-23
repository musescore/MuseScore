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

#include "cleflist.h"
#include "clef.h"
#include "measure.h"
#include "score.h"
#include "staff.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   ClefTypeList::operator==
//---------------------------------------------------------

bool ClefTypeList::operator==(const ClefTypeList& t) const
      {
      return t._concertClef == _concertClef && t._transposingClef == _transposingClef;
      }

//---------------------------------------------------------
//   ClefTypeList::operator!=
//---------------------------------------------------------

bool ClefTypeList::operator!=(const ClefTypeList& t) const
      {
      return t._concertClef != _concertClef || t._transposingClef != _transposingClef;
      }

//---------------------------------------------------------
//   ClefList
//---------------------------------------------------------

ClefList::ClefList(Staff *staff)
      {
      _staff = staff;
      // make sure that is at least 1 map item at tick 0
      _list.insert(std::pair<int, ClefTypeList>(0, ClefTypeList()));
      }
/*
// copy c'tor: do not simply copy data, but update score clefs too
ClefList::ClefList(const ClefList &other)
      {
//      _staff = other._staff;
      clear();
      for (auto i : other._list) {
            setClef(i.first, i.second);
            }
      }
*/
// assignment operator: do not simply copy data, but update score clefs too
ClefList& ClefList::operator=(const ClefList& other)
      {
      clear();
      for (auto i : other._list) {
            setClef(i.first, i.second);
            }
      return *this;
      }

ClefList::~ClefList()
      {
      _list.clear();
      }

//---------------------------------------------------------
//   ClefList::clef
//---------------------------------------------------------

ClefTypeList ClefList::clef(int tick) const
      {
      if (_list.empty() || tick < 0)
            return ClefTypeList(ClefType::INVALID, ClefType::INVALID);
      auto i = _list.upper_bound(tick);
      if (i == _list.begin())
            return ClefTypeList();
      return (--i)->second;
      }

//---------------------------------------------------------
//   ClefList::isClefChangeAt
//    returns true if there is a clef change at tick
//---------------------------------------------------------

bool ClefList::isClefChangeAt(int tick) const
      {
      if (_list.empty())
            return (tick == 0);           // there is always a clef set at 0 (possibly implicitly)
      return (_list.count(tick) > 0);
      }

//---------------------------------------------------------
//   ClefList::setClef
//---------------------------------------------------------

void ClefList::setClef(int tick, ClefTypeList ctl)
      {
      // locate the clef position in the score
      Measure* m = _staff->score()->tick2measure(tick);
      Segment* seg = nullptr;
      if (m) {
            // if new clef is at the beginning of measure,
            // move to previous measure (if any)
            if (tick == m->tick()) {
                  Measure* m2;
                  if ( (m2=m->prevMeasure()) != nullptr)
                        m = m2;
                  }
            seg = m->findSegment(Segment::SegClef, tick);
            }
      bool bAdd = true;                         // true = add or set clef into score
                                                // false = remove clef from score
      // if no clef change do nothing to list
      if (clef(tick) == ctl)
            ;
      // if clef would be the same as the clef in effect,
      // remove from list and from score
      else if (tick > 0 && clef(tick-1) == ctl) {
            _list.erase(tick);
            bAdd = false;
            }
      // if clef is really new
      else  {
            // add to map if not already there at this tick
            auto i = _list.find(tick);
            if (i == _list.end())
                  // insert in list
                  _list.insert(std::pair<int, ClefTypeList>(tick, ctl));
            // if clef change there already, update clef type
            else {
// DEBUG
if (i->second == ctl)
      qDebug("ClefList::setClef(): same clef already set.");
                  i->second = ctl;
                  }
            }

      // adjust score
      // (it is not executed during score file reading as the measure being read
      // is not inserted into the score linked chain of measures yet)
      if (m) {
            int track = _staff->idx() * VOICES;
            if (bAdd && seg == nullptr)
                  seg = m->getSegment(Segment::SegClef, tick);
            if (seg) {
                  Clef* clef = static_cast<Clef*>(seg->element(track));
                  // if adding/setting clef
                  if (bAdd) {
                        // if clef already there, set type and generated flag
                        if (seg->element(track)) {
                              clef->setClefType(ctl);
                              clef->setGenerated(false);
                              }
                        // if clef not there, add
                        else {
                              Clef* clef = new Clef(_staff->score());
                              clef->setClefType(ctl);
                              clef->setTrack(track);
                              clef->setParent(seg);
                              clef->setGenerated(false);
                              seg->add(clef);
                              }
                        }
                  // if removing clef
                  else {
                        if (clef != nullptr)
                              seg->removeElement(track);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   ClefList::clear
//---------------------------------------------------------

void ClefList::clear()
      {
      // remove from staff measures all non-generated clefs listed in the map
      int track = _staff->idx() * VOICES;
      for (auto i : _list) {
            Measure* msr = _staff->score()->tick2measure(i.first);
            Segment* seg = nullptr;
            if (msr) {
                  // look for a clef segment in this measure
                  seg = msr->findSegment(Segment::SegClef, i.first);
                  // if no clef segment, look in previous measure (if any)
                  if (seg == nullptr) {
                              msr = msr->prevMeasure();
                              if (msr)
                                    seg = msr->findSegment(Segment::SegClef, i.first);
                              }
                  // if segment found, remove this clef from it
                  if (seg)
                        seg->removeElement(track);
                  }
            }
      _list.clear();
      _list.insert(std::pair<int, ClefTypeList>(0, ClefTypeList()));
      }

//---------------------------------------------------------
//   ClefList::insertTime
//---------------------------------------------------------

void ClefList::insertTime(int tick, int len)
      {
      std::map<int, ClefTypeList> cl2;
      for (auto i = _list.upper_bound(tick); i != _list.end();) {
            ClefTypeList ctl = i->second;
            int key = i->first;
            _list.erase(i++);
            cl2.insert(std::pair<int, ClefTypeList>(key + len, ctl));
            }
      _list.insert(cl2.begin(), cl2.end());
      }

//---------------------------------------------------------
//   ClefList::read
//    only used for 1.3 scores
//---------------------------------------------------------

void ClefList::read114(XmlReader& e, Score* cs)
      {
      _list.clear();
      while (e.readNextStartElement()) {
            if (e.name() == "clef") {
                  int tick    = e.intAttribute("tick", 0);
                  ClefType ct = Clef::clefType(e.attribute("idx", "0"));
                  _list.insert(std::pair<int, ClefTypeList>(cs->fileDivision(tick), ClefTypeList(ct, ct)));
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   ClefList::alignScore
//    sets into the score the clef changes stored in the _list
//    only used for 1.3 scores
//---------------------------------------------------------

void ClefList::alignScore114()
      {
      Score* score = _staff->score();
      int    track = _staff->idx() * VOICES;

      for (auto i = _list.cbegin(); i != _list.cend(); ++i) {
          int tick = i->first;
          ClefType clefId = i->second._concertClef;
          Measure* m = score->tick2measure(tick);
          if (!m)
                continue;
          if ((tick == m->tick()) && m->prevMeasure())
                m = m->prevMeasure();
          Segment* seg = m->getSegment(Segment::SegClef, tick);
          if (seg->element(track))
                static_cast<Clef*>(seg->element(track))->setGenerated(false);
          else {
                Clef* clef = new Clef(score);
                clef->setClefType(clefId);
                clef->setTrack(track);
                clef->setParent(seg);
                clef->setGenerated(false);
                seg->add(clef);
                }
          }
      }
}
