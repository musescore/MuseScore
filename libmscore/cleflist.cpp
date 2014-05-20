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
      if (_list.empty())
            return ClefTypeList();
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
      if (clef(tick) == ctl)
            return;
      if (tick > 0 && clef(tick-1) == ctl)
            _list.erase(tick);
      else  {
            int track = _staff->idx() * VOICES;
            // if no clef change at this tick in map yet
            auto i = _list.find(tick);
            if (i == _list.end()) {
                  // insert in list
                  _list.insert(std::pair<int, ClefTypeList>(tick, ctl));
                  // and insert in score
                  // (it is not executed during score file reading
                  // as the measure being read is not inserted
                  // into the score linked chain of measures yet)
                  Measure* m = _staff->score()->tick2measure(tick);
                  if (m) {
//                        bool small = false;
                        // if new clef is at the beginning of measure
                        // check if it has to be moved to previous measure
                        if (tick == m->tick()) {
                              Measure* m2;
                              // if there is previous measure and it is not end-repeat
                              // nor section-end measure, move clef to previous measure
                              if ( (m2=m->prevMeasure()) != nullptr
                                          && !(m2->repeatFlags() & RepeatEnd)
                                          && m2->sectionBreak() == nullptr)
                                    m = m2;
                              }
                        Segment* seg = m->getSegment(Segment::SegClef, tick);
                        if (seg->element(track))
                              static_cast<Clef*>(seg->element(track))->setGenerated(false);
                        else {
                              Clef* clef = new Clef(_staff->score());
                              clef->setClefType(ctl);
                              clef->setTrack(track);
                              clef->setParent(seg);
                              clef->setGenerated(false);
                              seg->add(clef);
                              }
                        }
                  }
            // if clef change there already, update clef type
            else {
// DEBUG
if (i->second == ctl)
      qDebug("ClefList::setClef(): same clef already set.");
                  i->second = ctl;
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

void ClefList::read(XmlReader& e, Score* cs)
      {
      _list.clear();
      while (e.readNextStartElement()) {
            if (e.name() == "clef") {
                  int tick    = e.intAttribute("tick", 0);
                  ClefType ct = Clef::clefType(e.attribute("idx", "0"));
//                  _list.insert(std::pair<int, ClefTypeList>(cs->fileDivision(tick), ClefTypeList(ct, ct)));
                  setClef(cs->fileDivision(tick), ClefTypeList(ct, ct));
                  e.readNext();
                  }
            else
                  e.unknown();
            }
      }
}

