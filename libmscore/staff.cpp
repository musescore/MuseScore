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

#include "mscore.h"
#include "staff.h"
#include "part.h"
#include "clef.h"
#include "xml.h"
#include "score.h"
#include "bracket.h"
#include "keysig.h"
#include "segment.h"
#include "style.h"
#include "measure.h"
#include "stringdata.h"
#include "stafftype.h"
#include "undo.h"
#include "cleflist.h"
#include "timesig.h"
#include "instrtemplate.h"
#include "barline.h"
#include "ottava.h"
#include "harmony.h"
#include "bracketItem.h"

// #define DEBUG_CLEFS

#ifdef DEBUG_CLEFS
#define DUMP_CLEFS(s) dumpClefs(s)
#else
#define DUMP_CLEFS(s)
#endif

namespace Ms {

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff(Score* score)
   : ScoreElement(score)
      {
//      initFromStaffType(0);
      }

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Staff::idx() const
      {
      return score()->staves().indexOf((Staff*)this, 0);
      }

//---------------------------------------------------------
//   fillBrackets
//    make sure index idx is valid
//---------------------------------------------------------

void Staff::fillBrackets(int idx)
      {
      for (int i = _brackets.size(); i <= idx; ++i) {
            BracketItem* bi = new BracketItem(score());
            bi->setStaff(this);
            bi->setColumn(i);
            _brackets.append(bi);
            }
      }

//---------------------------------------------------------
//   cleanBrackets
//    remove NO_BRACKET entries from the end of list
//---------------------------------------------------------

void Staff::cleanBrackets()
      {
      while (!_brackets.empty() && (_brackets.last()->bracketType() == BracketType::NO_BRACKET))
            delete _brackets.takeLast();
      }

//---------------------------------------------------------
//   bracket
//---------------------------------------------------------

BracketType Staff::bracketType(int idx) const
      {
      if (idx < _brackets.size())
            return _brackets[idx]->bracketType();
      return BracketType::NO_BRACKET;
      }

//---------------------------------------------------------
//   bracketSpan
//---------------------------------------------------------

int Staff::bracketSpan(int idx) const
      {
      if (idx < _brackets.size())
            return _brackets[idx]->bracketSpan();
      return 0;
      }

//---------------------------------------------------------
//   setBracket
//---------------------------------------------------------

void Staff::setBracketType(int idx, BracketType val)
      {
      fillBrackets(idx);
      _brackets[idx]->setBracketType(val);
      cleanBrackets();
      }

//---------------------------------------------------------
//   swapBracket
//---------------------------------------------------------

void Staff::swapBracket(int oldIdx, int newIdx)
      {
      int idx = qMax(oldIdx, newIdx);
      fillBrackets(idx);
      _brackets[oldIdx]->setColumn(newIdx);
      _brackets[newIdx]->setColumn(oldIdx);
      _brackets.swap(oldIdx, newIdx);
      cleanBrackets();
      }

//---------------------------------------------------------
//   changeBracketColumn
//---------------------------------------------------------

void Staff::changeBracketColumn(int oldColumn, int newColumn)
      {
      int idx = qMax(oldColumn, newColumn);
      fillBrackets(idx);
      int step = newColumn > oldColumn ? 1 : -1;
      for (int i = oldColumn; i != newColumn; i += step) {
            int oldIdx = i;
            int newIdx = i + step;
            _brackets[oldIdx]->setColumn(newIdx);
            _brackets[newIdx]->setColumn(oldIdx);
            _brackets.swap(oldIdx, newIdx);
            }
      cleanBrackets();
      }

//---------------------------------------------------------
//   setBracketSpan
//---------------------------------------------------------

void Staff::setBracketSpan(int idx, int val)
      {
      Q_ASSERT(idx >= 0);
      Q_ASSERT(val >= 0);
      fillBrackets(idx);
      _brackets[idx]->setBracketSpan(val);
      }

//---------------------------------------------------------
//   addBracket
//---------------------------------------------------------

void Staff::addBracket(BracketItem* b)
      {
      b->setStaff(this);
      if (!_brackets.empty() && _brackets[0]->bracketType() == BracketType::NO_BRACKET)
            _brackets[0] = b;
      else {
            //
            // create new bracket level
            //
            for (Staff* s : score()->staves()) {
                  if (s == this)
                        s->_brackets.append(b);
                  else {
                        BracketItem* bi = new BracketItem(score());
                        bi->setStaff(this);
                        s->_brackets.append(bi);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   innerBracket
//    Return type inner bracket.
//    The bracket type determines the staff distance.
//---------------------------------------------------------

BracketType Staff::innerBracket() const
      {
      int staffIdx = idx();

      BracketType t = BracketType::NO_BRACKET;
      int level = 1000;
      for (int i = 0; i < score()->nstaves(); ++i) {
            Staff* staff = score()->staff(i);
            for (int k = 0; k < staff->brackets().size(); ++k) {
                  const BracketItem* bi = staff->brackets().at(k);
                  if (bi->bracketType() != BracketType::NO_BRACKET) {
                        if (i < staffIdx && ((i + bi->bracketSpan()) > staffIdx) && k < level) {
                              t = bi->bracketType();
                              level = k;
                              break;
                              }
                        }
                  }
            }
      return t;
      }

//---------------------------------------------------------
//   cleanupBrackets
//---------------------------------------------------------

void Staff::cleanupBrackets()
      {
      int index = idx();
      int n = score()->nstaves();
      for (int i = 0; i < _brackets.size(); ++i) {
            if (_brackets[i]->bracketType() == BracketType::NO_BRACKET)
                  continue;
            int span = _brackets[i]->bracketSpan();
            if (span > (n - index)) {
                  span = n - index;
                  _brackets[i]->setBracketSpan(span);
                  }
            }
      for (int i = 0; i < _brackets.size(); ++i) {
            if (_brackets[i]->bracketType() == BracketType::NO_BRACKET)
                  continue;
            int span = _brackets[i]->bracketSpan();
            if (span <= 1) {
                  _brackets[i] = new BracketItem(score());
                  _brackets[i]->setStaff(this);
                  }
            else {
                  // delete all other brackets with same span
                  for (int k = i + 1; k < _brackets.size(); ++k) {
                        if (span == _brackets[k]->bracketSpan()) {
                              _brackets[k] = new BracketItem(score());
                              _brackets[k]->setStaff(this);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   bracketLevels
//---------------------------------------------------------

int Staff::bracketLevels() const
      {
      int columns = 0;
      for (auto bi : _brackets)
           columns = qMax(columns, bi->column());
      return columns;
      }

//---------------------------------------------------------
//   partName
//---------------------------------------------------------

QString Staff::partName() const
      {
      return _part->partName();
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
#if 0
      if (_linkedStaves) {
            _linkedStaves->remove(this);
            if (_linkedStaves->empty())
                  delete _linkedStaves;
            }
#endif
      }

//---------------------------------------------------------
//   Staff::clefType
//---------------------------------------------------------

ClefTypeList Staff::clefType(int tick) const
      {
      ClefTypeList ct = clefs.clef(tick);
      if (ct._concertClef == ClefType::INVALID) {
            switch (staffType(tick)->group()) {
                  case StaffGroup::TAB:
                        {
                        ClefType sct = ClefType(score()->styleI(Sid::tabClef));
                        ct = staffType(tick)->lines() <= 4 ?  ClefTypeList(sct == ClefType::TAB ? ClefType::TAB4 : ClefType::TAB4_SERIF) : ClefTypeList(sct == ClefType::TAB ? ClefType::TAB : ClefType::TAB_SERIF);
                        }
                        break;
                  case StaffGroup::STANDARD:
                        ct = defaultClefType();
                        break;
                  case StaffGroup::PERCUSSION:
                        ct = ClefTypeList(ClefType::PERC);
                        break;
                  }
            }
      return ct;
      }

//---------------------------------------------------------
//   Staff::clef
//---------------------------------------------------------

ClefType Staff::clef(int tick) const
      {
      ClefTypeList c = clefType(tick);
      return score()->styleB(Sid::concertPitch) ? c._concertClef : c._transposingClef;
      }

//---------------------------------------------------------
//   Staff::nextClefTick
//
//    return the tick of next clef after tick
//    return last tick of score if not found
//---------------------------------------------------------

int Staff::nextClefTick(int tick) const
      {
      int t = clefs.nextClefTick(tick);
      return t != -1 ? t : score()->endTick();
      }


#ifndef NDEBUG
//---------------------------------------------------------
//   dumpClef
//---------------------------------------------------------

void Staff::dumpClefs(const char* title) const
      {
      qDebug("(%zd): %s", clefs.size(), title);
      for (auto& i : clefs) {
            qDebug("  %d: %d %d", i.first, int(i.second._concertClef), int(i.second._transposingClef));
            }
      }

//---------------------------------------------------------
//   dumpKeys
//---------------------------------------------------------

void Staff::dumpKeys(const char* title) const
      {
      qDebug("(%zd): %s", _keys.size(), title);
      for (auto& i : _keys) {
            qDebug("  %d: %d", i.first, int(i.second.key()));
            }
      }

//---------------------------------------------------------
//   dumpTimeSigs
//---------------------------------------------------------

void Staff::dumpTimeSigs(const char* title) const
      {
      qDebug("size (%zd) staffIdx %d: %s", timesigs.size(), idx(), title);
      for (auto& i : timesigs) {
            qDebug("  %d: %d/%d", i.first, i.second->sig().numerator(), i.second->sig().denominator());
            }
      }
#endif

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void Staff::setClef(Clef* clef)
      {
//      qDebug("Staff::setClef generated %d", clef->generated());
      if (clef->generated())
            return;
      int tick = clef->segment()->tick();
      for (Segment* s = clef->segment()->next(); s && s->tick() == tick; s = s->next()) {
            if (s->segmentType() == SegmentType::Clef && s->element(clef->track())) {
                  // adding this clef has no effect on the clefs list
                  return;
                  }
            }
      clefs.setClef(clef->segment()->tick(), clef->clefTypeList());
      DUMP_CLEFS("setClef");
      }

//---------------------------------------------------------
//   removeClef
//---------------------------------------------------------

void Staff::removeClef(Clef* clef)
      {
//      qDebug("Staff::removeClef generated %d", clef->generated());
      if (clef->generated())
            return;
      int tick = clef->segment()->tick();
      for (Segment* s = clef->segment()->next(); s && s->tick() == tick; s = s->next()) {
            if (s->segmentType() == SegmentType::Clef && s->element(clef->track())) {
                  // removal of this clef has no effect on the clefs list
                  return;
                  }
            }
      clefs.erase(clef->segment()->tick());
      for (Segment* s = clef->segment()->prev(); s && s->tick() == tick; s = s->prev()) {
            if (s->segmentType() == SegmentType::Clef
               && s->element(clef->track())
               && !s->element(clef->track())->generated()) {
                  // a previous clef at the same tick position gets valid
                  clefs.setClef(tick, toClef(s->element(clef->track()))->clefTypeList());
                  break;
                  }
            }
      DUMP_CLEFS("removeClef");
      }

//---------------------------------------------------------
//   timeStretch
//---------------------------------------------------------

Fraction Staff::timeStretch(int tick) const
      {
      TimeSig* timesig = timeSig(tick);
      return timesig ? timesig->stretch() : Fraction(1,1);
      }

//---------------------------------------------------------
//   timeSig
//    lookup time signature before or at tick
//---------------------------------------------------------

TimeSig* Staff::timeSig(int tick) const
      {
      auto i = timesigs.upper_bound(tick);
      if (i != timesigs.begin())
            --i;
      if (i == timesigs.end())
            return 0;
      else if (tick < i->first)
            return 0;
      return i->second;
      }

//---------------------------------------------------------
//   nextTimeSig
//    lookup time signature at tick or after
//---------------------------------------------------------

TimeSig* Staff::nextTimeSig(int tick) const
      {
      auto i = timesigs.lower_bound(tick);
      return (i == timesigs.end()) ? 0 : i->second;
      }

//---------------------------------------------------------
//   group
//---------------------------------------------------------

const Groups& Staff::group(int tick) const
      {
      TimeSig* ts = timeSig(tick);
      if (ts) {
            if (!ts->groups().empty())
                  return ts->groups();
            return Groups::endings(ts->sig());
            }
      Measure* m = score()->tick2measure(tick);
      return Groups::endings(m ? m->timesig() : Fraction(4,4));
      }

//---------------------------------------------------------
//   addTimeSig
//---------------------------------------------------------

void Staff::addTimeSig(TimeSig* timesig)
      {
      if (timesig->segment()->segmentType() == SegmentType::TimeSig)
            timesigs[timesig->segment()->tick()] = timesig;
//      dumpTimeSigs("after addTimeSig");
      }

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void Staff::removeTimeSig(TimeSig* timesig)
      {
      if (timesig->segment()->segmentType() == SegmentType::TimeSig)
            timesigs.erase(timesig->segment()->tick());
//      dumpTimeSigs("after removeTimeSig");
      }

//---------------------------------------------------------
//   clearTimeSig
//---------------------------------------------------------

void Staff::clearTimeSig()
      {
      timesigs.clear();
      }

//---------------------------------------------------------
//   Staff::keySigEvent
//
//    locates the key sig currently in effect at tick
//---------------------------------------------------------

KeySigEvent Staff::keySigEvent(int tick) const
      {
      return _keys.key(tick);
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void Staff::setKey(int tick, KeySigEvent k)
      {
      _keys.setKey(tick, k);
//    dumpKeys("setKey");
      }

//---------------------------------------------------------
//   removeKey
//---------------------------------------------------------

void Staff::removeKey(int tick)
      {
      _keys.erase(tick);
//    dumpKeys("removeKey");
      }

//---------------------------------------------------------
//   prevkey
//---------------------------------------------------------

KeySigEvent Staff::prevKey(int tick) const
      {
      return _keys.prevKey(tick);
      }

//---------------------------------------------------------
//   Staff::nextKeyTick
//
//    return the tick at which the key sig after tick is located
//    return 0, if no such a key sig
//---------------------------------------------------------

int Staff::nextKeyTick(int tick) const
      {
      int t = _keys.nextKeyTick(tick);
      return t != -1 ? t : score()->endTick();
      }

//---------------------------------------------------------
//   Staff::currentKeyTick
//
//    return the tick position of the key currently
//    in effect at tick
//    return 0, if no such a key sig
//---------------------------------------------------------

int Staff::currentKeyTick(int tick) const
      {
      return _keys.currentKeyTick(tick);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Staff::write(XmlWriter& xml) const
      {
      int idx = this->idx();
      xml.stag(this, QString("id=\"%1\"").arg(idx + 1));
      if (links()) {
            Score* s = masterScore();
            for (auto le : *links()) {
                  Staff* staff = toStaff(le);
                  if ((staff->score() == s) && (staff != this))
                        xml.tag("linkedTo", staff->idx() + 1);
                  }
            }

      // for copy/paste we need to know the actual transposition
      if (xml.clipboardmode()) {
            Interval v = part()->instrument()->transpose(); // TODO: tick?
            if (v.diatonic)
                  xml.tag("transposeDiatonic", v.diatonic);
            if (v.chromatic)
                  xml.tag("transposeChromatic", v.chromatic);
            }

      staffType(0)->write(xml);
      ClefTypeList ct = _defaultClefType;
      if (ct._concertClef == ct._transposingClef) {
            if (ct._concertClef != ClefType::G)
                  xml.tag("defaultClef", ClefInfo::tag(ct._concertClef));
            }
      else {
            xml.tag("defaultConcertClef", ClefInfo::tag(ct._concertClef));
            xml.tag("defaultTransposingClef", ClefInfo::tag(ct._transposingClef));
            }

      if (invisible())
            xml.tag("invisible", invisible());
      if (hideWhenEmpty() != HideMode::AUTO)
            xml.tag("hideWhenEmpty", int(hideWhenEmpty()));
      if (cutaway())
            xml.tag("cutaway", cutaway());
      if (showIfEmpty())
            xml.tag("showIfSystemEmpty", showIfEmpty());
      if (_hideSystemBarLine)
            xml.tag("hideSystemBarLine", _hideSystemBarLine);

      for (const BracketItem* i : _brackets) {
            BracketType a = i->bracketType();
            int b = i->bracketSpan();
            int c = i->column();
            if (a != BracketType::NO_BRACKET || b > 0)
                  xml.tagE(QString("bracket type=\"%1\" span=\"%2\" col=\"%3\"").arg((int)(a)).arg(b).arg(c));
            }

      writeProperty(xml, Pid::STAFF_BARLINE_SPAN);
      writeProperty(xml, Pid::STAFF_BARLINE_SPAN_FROM);
      writeProperty(xml, Pid::STAFF_BARLINE_SPAN_TO);
      writeProperty(xml, Pid::STAFF_USERDIST);
      writeProperty(xml, Pid::COLOR);
      writeProperty(xml, Pid::PLAYBACK_VOICE1);
      writeProperty(xml, Pid::PLAYBACK_VOICE2);
      writeProperty(xml, Pid::PLAYBACK_VOICE3);
      writeProperty(xml, Pid::PLAYBACK_VOICE4);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Staff::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());
      if (tag == "StaffType") {
            StaffType st;
            st.read(e);
            setStaffType(0, st);
            }
      else if (tag == "defaultClef") {           // sets both default transposing and concert clef
            QString val(e.readElementText());
            ClefType ct = Clef::clefType(val);
            setDefaultClefType(ClefTypeList(ct, ct));
            }
      else if (tag == "defaultConcertClef") {
            QString val(e.readElementText());
            setDefaultClefType(ClefTypeList(Clef::clefType(val), defaultClefType()._transposingClef));
            }
      else if (tag == "defaultTransposingClef") {
            QString val(e.readElementText());
            setDefaultClefType(ClefTypeList(defaultClefType()._concertClef, Clef::clefType(val)));
            }
      else if (tag == "small")                  // obsolete
            setSmall(0, e.readInt());
      else if (tag == "invisible")
            setInvisible(e.readInt());
      else if (tag == "hideWhenEmpty")
            setHideWhenEmpty(HideMode(e.readInt()));
      else if (tag == "cutaway")
            setCutaway(e.readInt());
      else if (tag == "showIfSystemEmpty")
            setShowIfEmpty(e.readInt());
      else if (tag == "hideSystemBarLine")
            _hideSystemBarLine = e.readInt();
      else if (tag == "keylist")
            _keys.read(e, score());
      else if (tag == "bracket") {
            int col = e.intAttribute("col", -1);
            if (col == -1)
                  col = _brackets.size();
            setBracketType(col, BracketType(e.intAttribute("type", -1)));
            setBracketSpan(col, e.intAttribute("span", 0));
            e.readNext();
            }
      else if (tag == "barLineSpan")
            _barLineSpan = e.readInt();
      else if (tag == "barLineSpanFrom")
            _barLineFrom = e.readInt();
      else if (tag == "barLineSpanTo")
            _barLineTo = e.readInt();
      else if (tag == "distOffset")
            _userDist = e.readDouble() * score()->spatium();
      else if (tag == "mag")
            /*_userMag =*/ e.readDouble(0.1, 10.0);
      else if (tag == "linkedTo") {
            int v = e.readInt() - 1;
            //
            // if this is an excerpt, link staff to masterScore()
            //
            if (!score()->isMaster()) {
                  Staff* st = masterScore()->staff(v);
                  if (st)
                        linkTo(st);
                  else {
                        qDebug("staff %d not found in parent", v);
                        }
                  }
            else {
                  if (v >= 0 && v < idx())
                        linkTo(score()->staff(v));
                  }
            }
      else if (tag == "color")
            _color = e.readColor();
      else if (tag == "transposeDiatonic")
            e.setTransposeDiatonic(e.readInt());
      else if (tag == "transposeChromatic")
            e.setTransposeChromatic(e.readInt());
      else if (tag == "playbackVoice1")
            setPlaybackVoice(0, e.readInt());
      else if (tag == "playbackVoice2")
            setPlaybackVoice(1, e.readInt());
      else if (tag == "playbackVoice3")
            setPlaybackVoice(2, e.readInt());
      else if (tag == "playbackVoice4")
            setPlaybackVoice(3, e.readInt());
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   height
//---------------------------------------------------------

qreal Staff::height() const
      {
      int tick = 0;     // TODO
//      return (lines(tick) == 1 ? 2 : lines(tick)-1) * spatium(tick) * staffType(tick)->lineDistance().val();
      return (lines(tick)-1) * spatium(tick) * staffType(tick)->lineDistance().val();
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Staff::spatium(int tick) const
      {
      return score()->spatium() * mag(tick);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Staff::mag(int tick) const
      {
      return (small(tick) ? score()->styleD(Sid::smallStaffMag) : 1.0) * userMag(tick);
      }

//---------------------------------------------------------
//   userMag
//---------------------------------------------------------

qreal Staff::userMag(int tick) const
      {
      return staffType(tick)->userMag();
      }

//---------------------------------------------------------
//   setUserMag
//---------------------------------------------------------

void Staff::setUserMag(int tick, qreal m)
      {
      staffType(tick)->setUserMag(m);
      }

//---------------------------------------------------------
//   small
//---------------------------------------------------------

bool Staff::small(int tick) const
      {
      return staffType(tick)->small();
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Staff::setSmall(int tick, bool val)
      {
      staffType(tick)->setSmall(val);
      }

//---------------------------------------------------------
//   swing
//---------------------------------------------------------

SwingParameters Staff::swing(int tick) const
      {
      SwingParameters sp;
      int swingUnit = 0;
      QString unit = score()->styleSt(Sid::swingUnit);
      int swingRatio = score()->styleI(Sid::swingRatio);
      if (unit == TDuration(TDuration::DurationType::V_EIGHTH).name()) {
            swingUnit = MScore::division / 2;
            }
      else if (unit == TDuration(TDuration::DurationType::V_16TH).name())
            swingUnit = MScore::division / 4;
      else if (unit == TDuration(TDuration::DurationType::V_ZERO).name())
            swingUnit = 0;
      sp.swingRatio = swingRatio;
      sp.swingUnit = swingUnit;
      if (_swingList.empty())
            return sp;
      QMap<int, SwingParameters>::const_iterator i = _swingList.upperBound(tick);
      if (i == _swingList.begin())
            return sp;
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   capo
//---------------------------------------------------------

int Staff::capo(int tick) const
      {
      if (_capoList.empty())
            return 0;
      QMap<int, int>::const_iterator i = _capoList.upperBound(tick);
      if (i == _capoList.begin())
            return 0;
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

int Staff::channel(int tick,  int voice) const
      {
      if (_channelList[voice].empty())
            return 0;
      QMap<int, int>::const_iterator i = _channelList[voice].upperBound(tick);
      if (i == _channelList[voice].begin())
            return 0;
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   middleLine
//    returns logical line number of middle staff line
//---------------------------------------------------------

int Staff::middleLine(int tick) const
      {
      return lines(tick) - 1;
      }

//---------------------------------------------------------
//   bottomLine
//    returns logical line number of bottom staff line
//---------------------------------------------------------

int Staff::bottomLine(int tick) const
      {
      return (lines(tick) - 1) * 2;
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Staff::slashStyle(int tick) const
      {
      return staffType(tick)->slashStyle();
      }

//---------------------------------------------------------
//   setSlashStyle
//---------------------------------------------------------

void Staff::setSlashStyle(int tick, bool val)
      {
      staffType(tick)->setSlashStyle(val);
      }

//---------------------------------------------------------
//   primaryStaff
///   if there are linked staves, the primary staff is
///   the one who is played back and it's not a tab staff
///   because we don't have enough information  to play
///   e.g ornaments. NOTE: it's not necessarily the top staff!
//---------------------------------------------------------

bool Staff::primaryStaff() const
      {
      if (!_links)
            return true;
      QList<Staff*> s;
      QList<Staff*> ss;
      for (auto e : *_links) {
            Staff* staff = toStaff(e);
            if (staff->score() == score()) {
                  s.append(staff);
                  if (!staff->isTabStaff(0))
                        ss.append(staff);
                  }
            }
      if (s.size() == 1) // the linked staves are in different scores
      	return s.front() == this;
      else // return a non tab linked staff in this score
      	return ss.front() == this;
      }

//---------------------------------------------------------
//   staffType
//---------------------------------------------------------

const StaffType* Staff::staffType(int tick) const
      {
      return &_staffTypeList.staffType(tick);
      }

const StaffType* Staff::constStaffType(int tick) const
      {
      return &_staffTypeList.staffType(tick);
      }

StaffType* Staff::staffType(int tick)
      {
      return &_staffTypeList.staffType(tick);
      }

//---------------------------------------------------------
//   staffTypeListChanged
//    Signal that the staffTypeList has changed at
//    position tick. Update layout range.
//---------------------------------------------------------

void Staff::staffTypeListChanged(int tick)
      {
      score()->setLayout(tick);
      auto i = _staffTypeList.find(tick);
      ++i;
      if (i != _staffTypeList.end())
            score()->setLayout(i->first);
      else
            score()->setLayout(score()->lastMeasure()->endTick());
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

StaffType* Staff::setStaffType(int tick, const StaffType& nst)
      {
      auto i = _staffTypeList.find(tick);
      if (i != _staffTypeList.end()) {
            qDebug("there is already a type at %d", tick);
            }
      return _staffTypeList.setStaffType(tick, nst);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const InstrumentTemplate* t, const StaffType* staffType, int cidx)
      {
      // set staff-type-independent parameters
      if (cidx >= MAX_STAVES) {
            setSmall(0, false);
            }
      else {
            setSmall(0,       t->smallStaff[cidx]);
            setBracketType(0, t->bracket[cidx]);
            setBracketSpan(0, t->bracketSpan[cidx]);
            setBarLineSpan(t->barlineSpan[cidx]);
            }
      const StaffType* pst = staffType ? staffType : t->staffTypePreset;
      if (!pst)
            pst = StaffType::getDefaultPreset(t->staffGroup);

      setStaffType(0, *pst);
      setDefaultClefType(t->clefType(cidx));
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const Staff* s)
      {
      _staffTypeList     = s->_staffTypeList;
      setDefaultClefType(s->defaultClefType());
      for (BracketItem* i : s->_brackets){
            BracketItem* ni = new BracketItem(*i);
            ni->setScore(score());
            ni->setStaff(this);
            _brackets.push_back(ni);
            }
      _barLineSpan       = s->_barLineSpan;
      _barLineFrom       = s->_barLineFrom;
      _barLineTo         = s->_barLineTo;
      _invisible         = s->_invisible;
      _hideWhenEmpty     = s->_hideWhenEmpty;
      _cutaway           = s->_cutaway;
      _showIfEmpty       = s->_showIfEmpty;
      _hideSystemBarLine = s->_hideSystemBarLine;
      _color             = s->_color;
      _userDist          = s->_userDist;
      }

//---------------------------------------------------------
//   initFromStaffType
//---------------------------------------------------------

void Staff::initFromStaffType(const StaffType* staffType)
      {
      // get staff type if given (if none, get default preset for default staff group)
      if (!staffType)
            staffType = StaffType::getDefaultPreset(StaffGroup::STANDARD);

      // use selected staff type
      setStaffType(0, *staffType);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Staff::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _userDist = (_userDist / oldValue) * newValue;
      }

//---------------------------------------------------------
//   show
//---------------------------------------------------------

bool Staff::show() const
      {
      return _part->show();
      }

//---------------------------------------------------------
//   genKeySig
//---------------------------------------------------------

bool Staff::genKeySig()
      {
      if (constStaffType(0)->group() == StaffGroup::TAB)
            return false;
      else
            return constStaffType(0)->genKeysig();
      }

//---------------------------------------------------------
//   showLedgerLines
//---------------------------------------------------------

bool Staff::showLedgerLines(int tick) const
      {
      return staffType(tick)->showLedgerLines();
      }

//---------------------------------------------------------
//   updateOttava
//---------------------------------------------------------

void Staff::updateOttava()
      {
      int staffIdx = idx();
      _pitchOffsets.clear();
      for (auto i : score()->spanner()) {
            const Spanner* s = i.second;
            if (s->type() == ElementType::OTTAVA && s->staffIdx() == staffIdx) {
                  const Ottava* o = static_cast<const Ottava*>(s);
                  _pitchOffsets.setPitchOffset(o->tick(), o->pitchShift());
                  _pitchOffsets.setPitchOffset(o->tick2(), 0);
                  }
            }
      }

//---------------------------------------------------------
//   undoSetColor
//---------------------------------------------------------

void Staff::undoSetColor(const QColor& /*val*/)
      {
//      undoChangeProperty(Pid::COLOR, val);
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Staff::insertTime(int tick, int len)
      {
      if (len == 0)
            return;

      // move all keys and clefs >= tick

      if (len < 0) {
            // remove entries between tickpos >= tick and tickpos < (tick+len)
            _keys.erase(_keys.lower_bound(tick), _keys.lower_bound(tick-len));
            clefs.erase(clefs.lower_bound(tick), clefs.lower_bound(tick-len));
            }

      KeyList kl2;
      for (auto i = _keys.lower_bound(tick); i != _keys.end();) {
            KeySigEvent kse = i->second;
            int t = i->first;
            _keys.erase(i++);
            kl2[t + len] = kse;
            }
      _keys.insert(kl2.begin(), kl2.end());

      // check if there is a clef at the end of measure
      // before tick
      Clef* clef = 0;
      Measure* m = score()->tick2measure(tick);
      if (m && (m->tick() == tick) && (m->prevMeasure())) {
            m = m->prevMeasure();
            Segment* s = m->findSegment(SegmentType::Clef, tick);
            if (s) {
                  int track = idx() * VOICES;
                  clef = toClef(s->element(track));
                  }
            }

      ClefList cl2;
      for (auto i = clefs.lower_bound(tick); i != clefs.end();) {
            ClefTypeList ctl = i->second;
            int t = i->first;
            if (clef && tick == t) {
                  ++i;
                  continue;
                  }
            clefs.erase(i++);
            cl2.setClef(t + len, ctl);
            }
      clefs.insert(cl2.begin(), cl2.end());

      // check if there is a clef at the end of measure
      // before tick: do not remove from clefs list

      if (clef)
            setClef(clef);

      updateOttava();
      DUMP_CLEFS("  insertTime");
      }

//---------------------------------------------------------
//   staffList
//    return list of linked staves
//---------------------------------------------------------

QList<Staff*> Staff::staffList() const
      {
      QList<Staff*> staffList;
      if (_links) {
            for (ScoreElement* e : *_links)
                  staffList.append(toStaff(e));
//            staffList = _linkedStaves->staves();
            }
      else
            staffList.append(const_cast<Staff*>(this));
      return staffList;
      }

//---------------------------------------------------------
//   rstaff
//---------------------------------------------------------

int Staff::rstaff() const
      {
      return _part->staves()->indexOf((Staff*)this, 0);
      }

//---------------------------------------------------------
//   isTop
//---------------------------------------------------------

bool Staff::isTop() const
      {
      return _part->staves()->front() == this;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Staff::getProperty(Pid id) const
      {
      switch (id) {
            case Pid::SMALL:
                  return small(0);
            case Pid::MAG:
                  return userMag(0);
            case Pid::COLOR:
                  return color();
            case Pid::PLAYBACK_VOICE1:
                  return playbackVoice(0);
            case Pid::PLAYBACK_VOICE2:
                  return playbackVoice(1);
            case Pid::PLAYBACK_VOICE3:
                  return playbackVoice(2);
            case Pid::PLAYBACK_VOICE4:
                  return playbackVoice(3);
            case Pid::STAFF_BARLINE_SPAN:
                  return barLineSpan();
            case Pid::STAFF_BARLINE_SPAN_FROM:
                  return barLineFrom();
            case Pid::STAFF_BARLINE_SPAN_TO:
                  return barLineTo();
            case Pid::STAFF_USERDIST:
                  return userDist();
            case Pid::GENERATED:
                  return false;
            default:
                  qDebug("unhandled id <%s>", propertyName(id));
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Staff::setProperty(Pid id, const QVariant& v)
      {
      switch (id) {
            case Pid::SMALL:
                  setSmall(0, v.toBool());
                  break;
            case Pid::MAG: {
                  qreal _spatium = spatium(0);
                  setUserMag(0, v.toReal());
                  score()->spatiumChanged(_spatium, spatium(0));
                  }
                  break;
            case Pid::COLOR:
                  setColor(v.value<QColor>());
                  break;
            case Pid::PLAYBACK_VOICE1:
                  setPlaybackVoice(0, v.toBool());
                  break;
            case Pid::PLAYBACK_VOICE2:
                  setPlaybackVoice(1, v.toBool());
                  break;
            case Pid::PLAYBACK_VOICE3:
                  setPlaybackVoice(2, v.toBool());
                  break;
            case Pid::PLAYBACK_VOICE4:
                  setPlaybackVoice(3, v.toBool());
                  break;
            case Pid::STAFF_BARLINE_SPAN:
                  setBarLineSpan(v.toInt());
                  break;
            case Pid::STAFF_BARLINE_SPAN_FROM:
                  setBarLineFrom(v.toInt());
                  break;
            case Pid::STAFF_BARLINE_SPAN_TO:
                  setBarLineTo(v.toInt());
                  break;
            case Pid::STAFF_USERDIST:
                  setUserDist(v.toReal());
                  break;
            default:
                  qDebug("unhandled id <%s>", propertyName(id));
                  break;
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Staff::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::SMALL:
                  return false;
            case Pid::MAG:
                  return 1.0;
            case Pid::COLOR:
                  return QColor(Qt::black);
            case Pid::PLAYBACK_VOICE1:
            case Pid::PLAYBACK_VOICE2:
            case Pid::PLAYBACK_VOICE3:
            case Pid::PLAYBACK_VOICE4:
                  return true;
            case Pid::STAFF_BARLINE_SPAN:
                  return false;
            case Pid::STAFF_BARLINE_SPAN_FROM:
            case Pid::STAFF_BARLINE_SPAN_TO:
                  return 0;
            case Pid::STAFF_USERDIST:
                  return qreal(0.0);
            default:
                  qDebug("unhandled id <%s>", propertyName(id));
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   scaleChanged
//---------------------------------------------------------

void Staff::scaleChanged(double oldVal, double newVal)
      {
      int staffIdx = idx();
      int startTrack = staffIdx * VOICES;
      int endTrack = startTrack + VOICES;
      for (Segment* s = score()->firstSegment(SegmentType::All); s; s = s->next1()) {
            for (Element* e : s->annotations())
                  e->localSpatiumChanged(oldVal, newVal);
            for (int track = startTrack; track < endTrack; ++track) {
                  if (s->element(track))
                        s->element(track)->localSpatiumChanged(oldVal, newVal);
                  }
            }
      for (auto i : score()->spanner()) {
            Spanner* spanner = i.second;
            if (spanner->staffIdx() == staffIdx) {
                  for (auto k : spanner->spannerSegments())
                        k->localSpatiumChanged(oldVal, newVal);
                  }
            }
      }

//---------------------------------------------------------
//   isPitchedStaff
//---------------------------------------------------------

bool Staff::isPitchedStaff(int tick) const
      {
      return staffType(tick)->group() == StaffGroup::STANDARD;
      }

//---------------------------------------------------------
//   isTabStaff
//---------------------------------------------------------

bool Staff::isTabStaff(int tick) const
      {
      return staffType(tick)->group() == StaffGroup::TAB;
      }

//---------------------------------------------------------
//   isDrumStaff
//---------------------------------------------------------

bool Staff::isDrumStaff(int tick) const
      {
      return staffType(tick)->group() == StaffGroup::PERCUSSION;
      }

//---------------------------------------------------------
//   lines
//---------------------------------------------------------

int Staff::lines(int tick) const
      {
      return staffType(tick)->lines();
      }

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void Staff::setLines(int tick, int val)
      {
      staffType(tick)->setLines(val);
      }

//---------------------------------------------------------
//   lineDistance
//    distance between staff lines
//---------------------------------------------------------

qreal Staff::lineDistance(int tick) const
      {
      return staffType(tick)->lineDistance().val();
      }

}

