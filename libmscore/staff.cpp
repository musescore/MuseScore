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
      initFromStaffType(0);
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
      while (!_brackets.empty() && (_brackets.last()->bracketType() == BracketType::NO_BRACKET)) {
            BracketItem* bi = _brackets.takeLast();
            delete bi;
            }
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
//   Staff::clefType
//---------------------------------------------------------

ClefTypeList Staff::clefType(const Fraction& tick) const
      {
      ClefTypeList ct = clefs.clef(tick.ticks());
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

ClefType Staff::clef(const Fraction& tick) const
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

Fraction Staff::nextClefTick(const Fraction& tick) const
      {
      Fraction t = Fraction::fromTicks(clefs.nextClefTick(tick.ticks()));
      return t != Fraction(-1,1) ? t : score()->endTick();
      }

//---------------------------------------------------------
//   Staff::currentClefTick
//
//    return the tick position of the clef currently
//    in effect at tick
//    return 0, if no such clef
//---------------------------------------------------------

Fraction Staff::currentClefTick(const Fraction& tick) const
      {
      return Fraction::fromTicks(clefs.currentClefTick(tick.ticks()));
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
      if (clef->generated())
            return;
      Fraction tick = clef->segment()->tick();
      for (Segment* s = clef->segment()->next1(); s && s->tick() == tick; s = s->next1()) {
            if ((s->segmentType() == SegmentType::Clef || s->segmentType() == SegmentType::HeaderClef)
                && s->element(clef->track())
                && !s->element(clef->track())->generated()) {
                  // adding this clef has no effect on the clefs list
                  return;
                  }
            }
      clefs.setClef(clef->segment()->tick().ticks(), clef->clefTypeList());
      DUMP_CLEFS("setClef");
      }

//---------------------------------------------------------
//   removeClef
//---------------------------------------------------------

void Staff::removeClef(const Clef* clef)
      {
      if (clef->generated())
            return;
      Fraction tick = clef->segment()->tick();
      for (Segment* s = clef->segment()->next1(); s && s->tick() == tick; s = s->next1()) {
            if ((s->segmentType() == SegmentType::Clef || s->segmentType() == SegmentType::HeaderClef)
                && s->element(clef->track())
                && !s->element(clef->track())->generated()) {
                  // removal of this clef has no effect on the clefs list
                  return;
                  }
            }
      clefs.erase(clef->segment()->tick().ticks());
      for (Segment* s = clef->segment()->prev1(); s && s->tick() == tick; s = s->prev1()) {
            if ((s->segmentType() == SegmentType::Clef || s->segmentType() == SegmentType::HeaderClef)
               && s->element(clef->track())
               && !s->element(clef->track())->generated()) {
                  // a previous clef at the same tick position gets valid
                  clefs.setClef(tick.ticks(), toClef(s->element(clef->track()))->clefTypeList());
                  break;
                  }
            }
      DUMP_CLEFS("removeClef");
      }

//---------------------------------------------------------
//   timeStretch
//---------------------------------------------------------

Fraction Staff::timeStretch(const Fraction& tick) const
      {
      TimeSig* timesig = timeSig(tick);
      return timesig ? timesig->stretch() : Fraction(1,1);
      }

//---------------------------------------------------------
//   timeSig
//    lookup time signature before or at tick
//---------------------------------------------------------

TimeSig* Staff::timeSig(const Fraction& tick) const
      {
      auto i = timesigs.upper_bound(tick.ticks());
      if (i != timesigs.begin())
            --i;
      if (i == timesigs.end())
            return 0;
      else if (tick < Fraction::fromTicks(i->first))
            return 0;
      return i->second;
      }

//---------------------------------------------------------
//   nextTimeSig
//    lookup time signature at tick or after
//---------------------------------------------------------

TimeSig* Staff::nextTimeSig(const Fraction& tick) const
      {
      auto i = timesigs.lower_bound(tick.ticks());
      return (i == timesigs.end()) ? 0 : i->second;
      }


//---------------------------------------------------------
//   currentTimeSigTick
//
//    return the tick position of the time sig currently
//    in effect at tick
//---------------------------------------------------------

Fraction Staff::currentTimeSigTick(const Fraction& tick) const
      {
      if (timesigs.empty())
            return Fraction(0, 1);
      auto i = timesigs.upper_bound(tick.ticks());
      if (i == timesigs.begin())
            return Fraction(0, 1);
      --i;
      return Fraction::fromTicks(i->first);
      }

//---------------------------------------------------------
//   group
//---------------------------------------------------------

const Groups& Staff::group(const Fraction& tick) const
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
            timesigs[timesig->segment()->tick().ticks()] = timesig;
//      dumpTimeSigs("after addTimeSig");
      }

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void Staff::removeTimeSig(TimeSig* timesig)
      {
      if (timesig->segment()->segmentType() == SegmentType::TimeSig)
            timesigs.erase(timesig->segment()->tick().ticks());
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

KeySigEvent Staff::keySigEvent(const Fraction& tick) const
      {
      return _keys.key(tick.ticks());
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void Staff::setKey(const Fraction& tick, KeySigEvent k)
      {
      _keys.setKey(tick.ticks(), k);
      }

//---------------------------------------------------------
//   removeKey
//---------------------------------------------------------

void Staff::removeKey(const Fraction& tick)
      {
      _keys.erase(tick.ticks());
      }

//---------------------------------------------------------
//   prevkey
//---------------------------------------------------------

KeySigEvent Staff::prevKey(const Fraction& tick) const
      {
      return _keys.prevKey(tick.ticks());
      }

//---------------------------------------------------------
//   Staff::nextKeyTick
//
//    return the tick at which the key sig after tick is located
//    return 0, if no such a key sig
//---------------------------------------------------------

Fraction Staff::nextKeyTick(const Fraction& tick) const
      {
      Fraction t = Fraction::fromTicks(_keys.nextKeyTick(tick.ticks()));
      return t != Fraction(-1,1) ? t : score()->endTick();
      }

//---------------------------------------------------------
//   Staff::currentKeyTick
//
//    return the tick position of the key currently
//    in effect at tick
//    return 0, if no such a key sig
//---------------------------------------------------------

Fraction Staff::currentKeyTick(const Fraction& tick) const
      {
      return Fraction::fromTicks(_keys.currentKeyTick(tick.ticks()));
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

      staffType(Fraction(0,1))->write(xml);
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
            setStaffType(Fraction(0,1), st);
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
            setSmall(Fraction(0,1), e.readInt());
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
            Staff* st = masterScore()->staff(v);
            if (_links) {
                  qDebug("Staff::readProperties: multiple <linkedTo> tags");
                  if (!st || isLinked(st)) // maybe we don't need actually to relink...
                        return true;
                  // not using unlink() here as it may delete _links
                  // a pointer to which is stored also in XmlReader.
                  _links->removeOne(this);
                  _links = nullptr;
                  }
            if (st && st != this)
                  linkTo(st);
            else if (!score()->isMaster() && !st) {
                  // if it is a master score it is OK not to find
                  // a staff which is going after the current one.
                  qDebug("staff %d not found in parent", v);
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
      Fraction tick = Fraction(0,1);     // TODO
//      return (lines(tick) == 1 ? 2 : lines(tick)-1) * spatium(tick) * staffType(tick)->lineDistance().val();
      return (lines(tick)-1) * spatium(tick) * staffType(tick)->lineDistance().val();
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Staff::spatium(const Fraction& tick) const
      {
      return score()->spatium() * mag(tick);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Staff::mag(const Fraction& tick) const
      {
      return (small(tick) ? score()->styleD(Sid::smallStaffMag) : 1.0) * userMag(tick);
      }

//---------------------------------------------------------
//   userMag
//---------------------------------------------------------

qreal Staff::userMag(const Fraction& tick) const
      {
      return staffType(tick)->userMag();
      }

//---------------------------------------------------------
//   setUserMag
//---------------------------------------------------------

void Staff::setUserMag(const Fraction& tick, qreal m)
      {
      staffType(tick)->setUserMag(m);
      }

//---------------------------------------------------------
//   small
//---------------------------------------------------------

bool Staff::small(const Fraction& tick) const
      {
      return staffType(tick)->small();
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void Staff::setSmall(const Fraction& tick, bool val)
      {
      staffType(tick)->setSmall(val);
      }

//---------------------------------------------------------
//   swing
//---------------------------------------------------------

SwingParameters Staff::swing(const Fraction& tick) const
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
      QMap<int, SwingParameters>::const_iterator i = _swingList.upperBound(tick.ticks());
      if (i == _swingList.begin())
            return sp;
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   capo
//---------------------------------------------------------

int Staff::capo(const Fraction& tick) const
      {
      if (_capoList.empty())
            return 0;
      QMap<int, int>::const_iterator i = _capoList.upperBound(tick.ticks());
      if (i == _capoList.begin())
            return 0;
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

int Staff::channel(const Fraction& tick,  int voice) const
      {
      if (_channelList[voice].empty())
            return 0;
      QMap<int, int>::const_iterator i = _channelList[voice].upperBound(tick.ticks());
      if (i == _channelList[voice].begin())
            return 0;
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   middleLine
//    returns logical line number of middle staff line
//---------------------------------------------------------

int Staff::middleLine(const Fraction& tick) const
      {
      return lines(tick) - 1;
      }

//---------------------------------------------------------
//   bottomLine
//    returns logical line number of bottom staff line
//---------------------------------------------------------

int Staff::bottomLine(const Fraction& tick) const
      {
      return (lines(tick) - 1) * 2;
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Staff::slashStyle(const Fraction& tick) const
      {
      return staffType(tick)->slashStyle();
      }

//---------------------------------------------------------
//   setSlashStyle
//---------------------------------------------------------

void Staff::setSlashStyle(const Fraction& tick, bool val)
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
                  if (!staff->isTabStaff(Fraction(0,1)))
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

const StaffType* Staff::staffType(const Fraction& tick) const
      {
      return &_staffTypeList.staffType(tick);
      }

const StaffType* Staff::constStaffType(const Fraction& tick) const
      {
      return &_staffTypeList.staffType(tick);
      }

StaffType* Staff::staffType(const Fraction& tick)
      {
      return &_staffTypeList.staffType(tick);
      }

//---------------------------------------------------------
//   staffTypeListChanged
//    Signal that the staffTypeList has changed at
//    position tick. Update layout range.
//---------------------------------------------------------

void Staff::staffTypeListChanged(const Fraction& tick)
      {
      score()->setLayout(tick);
      auto i = _staffTypeList.find(tick.ticks());
      if (i == _staffTypeList.end()) {
            score()->setLayoutAll();
            }
      else {
            ++i;
            if (i != _staffTypeList.end())
                  score()->setLayout(Fraction::fromTicks(i->first));
            else
                  score()->setLayout(score()->lastMeasure()->endTick());
            }
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

StaffType* Staff::setStaffType(const Fraction& tick, const StaffType& nst)
      {
      return _staffTypeList.setStaffType(tick, nst);
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

void Staff::removeStaffType(const Fraction& tick)
      {
      auto i = _staffTypeList.find(tick.ticks());
      if (i == _staffTypeList.end())
            return;
      qreal old = spatium(tick);
      _staffTypeList.erase(i);
      localSpatiumChanged(old, spatium(tick), tick);
      staffTypeListChanged(tick);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const InstrumentTemplate* t, const StaffType* staffType, int cidx)
      {
      // set staff-type-independent parameters
      const StaffType* pst = staffType ? staffType : t->staffTypePreset;
      if (!pst)
            pst = StaffType::getDefaultPreset(t->staffGroup);

      setStaffType(Fraction(0,1), *pst);
      if (cidx >= MAX_STAVES) {
            setSmall(Fraction(0,1), false);
            }
      else {
            setSmall(Fraction(0,1),       t->smallStaff[cidx]);
            setBracketType(0, t->bracket[cidx]);
            setBracketSpan(0, t->bracketSpan[cidx]);
            setBarLineSpan(t->barlineSpan[cidx]);
            }
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
      setStaffType(Fraction(0,1), *staffType);
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
      if (constStaffType(Fraction(0,1))->group() == StaffGroup::TAB)
            return false;
      else
            return constStaffType(Fraction(0,1))->genKeysig();
      }

//---------------------------------------------------------
//   showLedgerLines
//---------------------------------------------------------

bool Staff::showLedgerLines(const Fraction& tick) const
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
                  _pitchOffsets.setPitchOffset(o->tick().ticks(), o->pitchShift());
                  _pitchOffsets.setPitchOffset(o->tick2().ticks(), 0);
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

void Staff::insertTime(const Fraction& tick, const Fraction& len)
      {
      if (len.isZero())
            return;

      // move all keys and clefs >= tick

      if (len < Fraction(0,1)) {
            // remove entries between tickpos >= tick and tickpos < (tick+len)
            _keys.erase(_keys.lower_bound(tick.ticks()), _keys.lower_bound((tick - len).ticks()));
            clefs.erase(clefs.lower_bound(tick.ticks()), clefs.lower_bound((tick - len).ticks()));
            }

      KeyList kl2;
      for (auto i = _keys.lower_bound(tick.ticks()); i != _keys.end();) {
            KeySigEvent kse = i->second;
            Fraction t = Fraction::fromTicks(i->first);
            _keys.erase(i++);
            kl2[(t + len).ticks()] = kse;
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
      for (auto i = clefs.lower_bound(tick.ticks()); i != clefs.end();) {
            ClefTypeList ctl = i->second;
            Fraction t = Fraction::fromTicks(i->first);
            if (clef && tick == t) {
                  ++i;
                  continue;
                  }
            clefs.erase(i++);
            cl2.setClef((t + len).ticks(), ctl);
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
                  return small(Fraction(0,1));
            case Pid::MAG:
                  return userMag(Fraction(0,1));
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
            case Pid::SMALL: {
                  qreal _spatium = spatium(Fraction(0,1));
                  setSmall(Fraction(0,1), v.toBool());
                  localSpatiumChanged(_spatium, spatium(Fraction(0,1)), Fraction(0, 1));
                  break;
                  }
            case Pid::MAG: {
                  qreal _spatium = spatium(Fraction(0,1));
                  setUserMag(Fraction(0,1), v.toReal());
                  localSpatiumChanged(_spatium, spatium(Fraction(0,1)), Fraction(0, 1));
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
            case Pid::STAFF_BARLINE_SPAN: {
                  setBarLineSpan(v.toInt());
                  // update non-generated barlines
                  int track = idx() * VOICES;
                  std::vector<Element*> blList;
                  for (Measure* m = score()->firstMeasure(); m; m = m->nextMeasure()) {
                        Segment* s = m->getSegmentR(SegmentType::EndBarLine, m->ticks());
                        if (s && s->element(track))
                              blList.push_back(s->element(track));
                        if (Measure* mm = m->mmRest()) {
                              Segment* ss = mm->getSegmentR(SegmentType::EndBarLine, mm->ticks());
                              if (ss && ss->element(track))
                                    blList.push_back(ss->element(track));
                              }
                        }
                  for (Element* e : blList) {
                        if (e && e->isBarLine() && !e->generated())
                              toBarLine(e)->setSpanStaff(v.toInt());
                        }
                  }
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
//   localSpatiumChanged
//---------------------------------------------------------

void Staff::localSpatiumChanged(double oldVal, double newVal, Fraction tick)
      {
      Fraction etick;
      auto i = _staffTypeList.find(tick.ticks());
      ++i;
      if (i == _staffTypeList.end())
            etick = score()->lastSegment()->tick();
      else
            etick = Fraction::fromTicks(i->first);
      int staffIdx = idx();
      int startTrack = staffIdx * VOICES;
      int endTrack = startTrack + VOICES;
      for (Segment* s = score()->tick2rightSegment(tick); s && s->tick() < etick; s = s->next1()) {
            for (Element* e : s->annotations()) {
                  if (e->track() >= startTrack && e->track() < endTrack)
                        e->localSpatiumChanged(oldVal, newVal);
                  }
            for (int track = startTrack; track < endTrack; ++track) {
                  if (s->element(track))
                        s->element(track)->localSpatiumChanged(oldVal, newVal);
                  }
            }
      auto spanners = score()->spannerMap().findContained(tick.ticks(), etick.ticks());
      for (auto interval : spanners) {
            Spanner* spanner = interval.value;
            if (spanner->staffIdx() == staffIdx) {
                  for (auto k : spanner->spannerSegments())
                        k->localSpatiumChanged(oldVal, newVal);
                  }
            }
      }

//---------------------------------------------------------
//   isPitchedStaff
//---------------------------------------------------------

bool Staff::isPitchedStaff(const Fraction& tick) const
      {
      return staffType(tick)->group() == StaffGroup::STANDARD;
      }

//---------------------------------------------------------
//   isTabStaff
//---------------------------------------------------------

bool Staff::isTabStaff(const Fraction& tick) const
      {
      return staffType(tick)->group() == StaffGroup::TAB;
      }

//---------------------------------------------------------
//   isDrumStaff
//---------------------------------------------------------

bool Staff::isDrumStaff(const Fraction& tick) const
      {
      return staffType(tick)->group() == StaffGroup::PERCUSSION;
      }

//---------------------------------------------------------
//   lines
//---------------------------------------------------------

int Staff::lines(const Fraction& tick) const
      {
      return staffType(tick)->lines();
      }

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void Staff::setLines(const Fraction& tick, int val)
      {
      staffType(tick)->setLines(val);
      }

//---------------------------------------------------------
//   lineDistance
//    distance between staff lines
//---------------------------------------------------------

qreal Staff::lineDistance(const Fraction& tick) const
      {
      return staffType(tick)->lineDistance().val();
      }

}

