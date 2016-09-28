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

// #define DEBUG_CLEFS

#ifdef DEBUG_CLEFS
#define DUMP_CLEFS(s) dumpClefs(s)
#else
#define DUMP_CLEFS(s)
#endif

namespace Ms {

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Staff::idx() const
      {
      return score()->staves().indexOf((Staff*)this, 0);
      }

//---------------------------------------------------------
//   bracket
//---------------------------------------------------------

BracketType Staff::bracket(int idx) const
      {
      if (idx < _brackets.size())
            return _brackets[idx]._bracket;
      return BracketType::NO_BRACKET;
      }

//---------------------------------------------------------
//   bracketSpan
//---------------------------------------------------------

int Staff::bracketSpan(int idx) const
      {
      if (idx < _brackets.size())
            return _brackets[idx]._bracketSpan;
      return 0;
      }

//---------------------------------------------------------
//   setBracket
//---------------------------------------------------------

void Staff::setBracket(int idx, BracketType val)
      {
      for (int i = _brackets.size(); i <= idx; ++i)
            _brackets.append(BracketItem());
      _brackets[idx]._bracket = val;
      while (!_brackets.empty() && (_brackets.last()._bracket == BracketType::NO_BRACKET))
            _brackets.removeLast();
      }

//---------------------------------------------------------
//   setBracketSpan
//---------------------------------------------------------

void Staff::setBracketSpan(int idx, int val)
      {
      Q_ASSERT(idx >= 0);
      Q_ASSERT(val >= 0);
      for (int i = _brackets.size(); i <= idx; ++i)
            _brackets.append(BracketItem());
      _brackets[idx]._bracketSpan = val;
      }

//---------------------------------------------------------
//   addBracket
//---------------------------------------------------------

void Staff::addBracket(BracketItem b)
      {
      if (!_brackets.empty() && _brackets[0]._bracket == BracketType::NO_BRACKET) {
            _brackets[0] = b;
            }
      else {
            //
            // create new bracket level
            //
            foreach(Staff* s, score()->staves()) {
                  if (s == this)
                        s->_brackets.append(b);
                  else
                        s->_brackets.append(BracketItem());
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
                  const BracketItem& bi = staff->brackets().at(k);
                  if (bi._bracket != BracketType::NO_BRACKET) {
                        if (i < staffIdx && ((i + bi._bracketSpan) > staffIdx) && k < level) {
                              t = bi._bracket;
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
            if (_brackets[i]._bracket == BracketType::NO_BRACKET)
                  continue;
            int span = _brackets[i]._bracketSpan;
            if (span > (n - index)) {
                  span = n - index;
                  _brackets[i]._bracketSpan = span;
                  }
            }
      for (int i = 0; i < _brackets.size(); ++i) {
            if (_brackets[i]._bracket == BracketType::NO_BRACKET)
                  continue;
            int span = _brackets[i]._bracketSpan;
            if (span <= 1)
                  _brackets[i] = BracketItem();
            else {
                  // delete all other brackets with same span
                  for (int k = i + 1; k < _brackets.size(); ++k) {
                        if (span == _brackets[k]._bracketSpan)
                              _brackets[k] = BracketItem();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   partName
//---------------------------------------------------------

QString Staff::partName() const
      {
      return _part->partName();
      }

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff(Score* s)
  : ScoreElement(s)
      {
      _barLineTo = (lines() - 1) * 2;
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
      if (_linkedStaves) {
            _linkedStaves->remove(this);
            if (_linkedStaves->empty())
                  delete _linkedStaves;
            }
      }

//---------------------------------------------------------
//   Staff::clefType
//---------------------------------------------------------

ClefTypeList Staff::clefType(int tick) const
      {
      ClefTypeList ct = clefs.clef(tick);
      if (ct._concertClef == ClefType::INVALID) {
            switch(_staffType.group()) {
                  case StaffGroup::TAB:
                        {
                        ClefType sct = ClefType(score()->styleI(StyleIdx::tabClef));
                        ct = _staffType.lines() <= 4 ?  ClefTypeList(sct == ClefType::TAB ? ClefType::TAB4 : ClefType::TAB4_SERIF) : ClefTypeList(sct == ClefType::TAB ? ClefType::TAB : ClefType::TAB_SERIF);
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
      return score()->styleB(StyleIdx::concertPitch) ? c._concertClef : c._transposingClef;
      }

#ifndef NDEBUG
//---------------------------------------------------------
//   dumpClef
//---------------------------------------------------------

void Staff::dumpClefs(const char* title) const
      {
      qDebug("dump clefs (%zd): %s", clefs.size(), title);
      for (auto& i : clefs) {
            qDebug("  %d: %d %d", i.first, int(i.second._concertClef), int(i.second._transposingClef));
            }
      }

//---------------------------------------------------------
//   dumpKeys
//---------------------------------------------------------

void Staff::dumpKeys(const char* title) const
      {
      qDebug("dump keys (%zd): %s", _keys.size(), title);
      for (auto& i : _keys) {
            qDebug("  %d: %d", i.first, int(i.second.key()));
            }
      }

//---------------------------------------------------------
//   dumpTimeSigs
//---------------------------------------------------------

void Staff::dumpTimeSigs(const char* title) const
      {
      qDebug("dump timesig size (%zd) staffIdx %d: %s", timesigs.size(), idx(), title);
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
            if (s->segmentType() == Segment::Type::Clef && s->element(clef->track())) {
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
            if (s->segmentType() == Segment::Type::Clef && s->element(clef->track())) {
                  // removal of this clef has no effect on the clefs list
                  return;
                  }
            }
      clefs.erase(clef->segment()->tick());
      for (Segment* s = clef->segment()->prev(); s && s->tick() == tick; s = s->prev()) {
            if (s->segmentType() == Segment::Type::Clef
               && s->element(clef->track())
               && !s->element(clef->track())->generated()) {
                  // a previous clef at the same tick position gets valid
                  clefs.setClef(tick, static_cast<Clef*>(s->element(clef->track()))->clefTypeList());
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
      return timesig == 0 ? Fraction(1,1) : timesig->stretch();
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
      else if (tick < i->first)
            return 0;
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
      if (timesig->segment()->segmentType() == Segment::Type::TimeSig)
            timesigs[timesig->segment()->tick()] = timesig;
//      dumpTimeSigs("after addTimeSig");
      }

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void Staff::removeTimeSig(TimeSig* timesig)
      {
      if (timesig->segment()->segmentType() == Segment::Type::TimeSig)
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
      return _keys.nextKeyTick(tick);
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

void Staff::write(Xml& xml) const
      {
      int idx = this->idx();
      xml.stag(QString("Staff id=\"%1\"").arg(idx + 1));
      if (linkedStaves()) {
            Score* s = masterScore();
            foreach(Staff* staff, linkedStaves()->staves()) {
                  if ((staff->score() == s) && (staff != this))
                        xml.tag("linkedTo", staff->idx() + 1);
                  }
            }

      // for copy/paste we need to know the actual transposition
      if (xml.clipboardmode) {
            Interval v = part()->instrument()->transpose(); // TODO: tick?
            if (v.diatonic)
                  xml.tag("transposeDiatonic", v.diatonic);
            if (v.chromatic)
                  xml.tag("transposeChromatic", v.chromatic);
            }

      _staffType.write(xml);
      ClefTypeList ct = _defaultClefType;
      if (ct._concertClef == ct._transposingClef) {
            if (ct._concertClef != ClefType::G)
                  xml.tag("defaultClef", ClefInfo::tag(ct._concertClef));
            }
      else {
            xml.tag("defaultConcertClef", ClefInfo::tag(ct._concertClef));
            xml.tag("defaultTransposingClef", ClefInfo::tag(ct._transposingClef));
            }

      if (small() && !xml.excerptmode)    // switch small staves to normal ones when extracting part
            xml.tag("small", small());
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

      for (const BracketItem& i : _brackets)
            xml.tagE(QString("bracket type=\"%1\" span=\"%2\"").arg((signed char)(i._bracket)).arg(i._bracketSpan));

      // for economy and consistency, only output "from" and "to" attributes if different from default
      int defaultLineFrom = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
      int defaultLineTo;
      if (_barLineSpan == 0)                    // if no bar line at all
            defaultLineTo = _barLineTo;         // whatever the current spanTo is, use as default
      else {                                    // if some bar line, default is the default for span target staff
            int targetStaffIdx = idx + _barLineSpan - 1;
            if (targetStaffIdx >= score()->nstaves()) {
                  qInfo("bad _barLineSpan %d for staff %d (nstaves %d)",
                     _barLineSpan, idx, score()->nstaves());
                  targetStaffIdx = score()->nstaves() - 1;
                  }
            int targetStaffLines = score()->staff(targetStaffIdx)->lines();
            defaultLineTo = (targetStaffLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (targetStaffLines-1) * 2);
            }
      if (_barLineSpan != 1 || _barLineFrom != defaultLineFrom || _barLineTo != defaultLineTo) {
            if (_barLineFrom != defaultLineFrom || _barLineTo != defaultLineTo)
                  xml.tag(QString("barLineSpan from=\"%1\" to=\"%2\"").arg(_barLineFrom).arg(_barLineTo), _barLineSpan);
            else
                  xml.tag("barLineSpan", _barLineSpan);
            }
      if (_userDist != 0.0)
            xml.tag("distOffset", _userDist / score()->spatium());

      writeProperty(xml, P_ID::MAG);
      writeProperty(xml, P_ID::COLOR);
      writeProperty(xml, P_ID::PLAYBACK_VOICE1);
      writeProperty(xml, P_ID::PLAYBACK_VOICE2);
      writeProperty(xml, P_ID::PLAYBACK_VOICE3);
      writeProperty(xml, P_ID::PLAYBACK_VOICE4);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "type") {    // obsolete
                  int staffTypeIdx = e.readInt();
                  qDebug("obsolete: Staff::read staffTypeIdx %d", staffTypeIdx);
                  _staffType = *StaffType::preset(StaffTypes(staffTypeIdx));
                  // set default barLineFrom and barLineTo according to staff type num. of lines
                  // (1-line staff bar lines are special)
                  _barLineFrom = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
                  _barLineTo   = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_TO   : (lines() - 1) * 2);
                  }
            else if (tag == "StaffType") {
                  _staffType.read(e);
                  // set default barLineFrom and barLineTo according to staff type num. of lines
                  // (1-line staff bar lines are special)
                  _barLineFrom = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
                  _barLineTo   = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_TO   : (lines() - 1) * 2);
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
            else if (tag == "small")
                  setSmall(e.readInt());
            else if (tag == "invisible")
                  setInvisible(e.readInt());
            else if (tag == "hideWhenEmpty")
                  setHideWhenEmpty(HideMode(e.readInt()));
            else if (tag == "neverHide") {      // 2.0 compatibility
                  bool v = e.readInt();
                  if (v)
                        setHideWhenEmpty(HideMode::NEVER);
                  }
            else if (tag == "cutaway")
                  setCutaway(e.readInt());
            else if (tag == "showIfSystemEmpty")
                  setShowIfEmpty(e.readInt());
            else if (tag == "hideSystemBarLine")
                  _hideSystemBarLine = e.readInt();
            else if (tag == "keylist")
                  _keys.read(e, score());
            else if (tag == "bracket") {
                  BracketItem b;
                  b._bracket     = BracketType(e.intAttribute("type", -1));
                  b._bracketSpan = e.intAttribute("span", 0);
                  _brackets.append(b);
                  e.readNext();
                  }
            else if (tag == "barLineSpan") {
// WARNING: following statement assumes number of staff lines to be correctly set
                  // must read <StaffType> before reading the <barLineSpan>
                  int defaultSpan = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
                  _barLineFrom = e.intAttribute("from", defaultSpan);

                  // the proper default SpanTo depends upon the barLineSpan
                  // as we do not know it yet, set a generic (UNKNOWN) default
                  defaultSpan = UNKNOWN_BARLINE_TO;
                  _barLineTo = e.intAttribute("to", defaultSpan);

                  // ready to read the main value...
                  _barLineSpan = e.readInt();

                  //...and to adjust the SpanTo value if the source did not provide an explicit value
                  // if no bar line or single staff span, set _barLineTo to this staff height
                  // if span to another staff (yet to be read), leave as unknown
                  // (Score::read() will retrieve the correct height of the target staff)
                  if (_barLineTo == UNKNOWN_BARLINE_TO && _barLineSpan <= 1)
                        _barLineTo = lines() == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (lines() - 1) * 2;
                  }
            else if (tag == "distOffset")
                  _userDist = e.readDouble() * score()->spatium();
            else if (tag == "mag")
                  _userMag = e.readDouble(0.1, 10.0);
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
                        int idx = this->idx();
                        if (v >= 0 && v < idx)
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
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   height
//---------------------------------------------------------

qreal Staff::height() const
      {
      return (lines() == 1 ? 2 : lines()-1) * spatium() * _staffType.lineDistance().val();
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Staff::spatium() const
      {
      return score()->spatium() * mag();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Staff::mag() const
      {
      return (_small ? score()->styleD(StyleIdx::smallStaffMag) : 1.0) * userMag();
      }

//---------------------------------------------------------
//   swing
//---------------------------------------------------------

SwingParameters Staff::swing(int tick) const
      {
      SwingParameters sp;
      int swingUnit = 0;
      QString unit = score()->styleSt(StyleIdx::swingUnit);
      int swingRatio = score()->styleI(StyleIdx::swingRatio);
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
//   lines
//---------------------------------------------------------

int Staff::lines() const
      {
      return _staffType.lines();
      }

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void Staff::setLines(int val)
      {
      if (val == lines())
            return;
      _staffType.setLines(val);     // TODO: make undoable
      }

//---------------------------------------------------------
//   lineDistance
//    distance between staff lines
//---------------------------------------------------------

qreal Staff::lineDistance() const
      {
      return _staffType.lineDistance().val();
      }

//---------------------------------------------------------
//   logicalLineDistance
//    distance between logical (note) lines
//---------------------------------------------------------

qreal Staff::logicalLineDistance() const
      {
      return scaleNotesToLines() ? _staffType.lineDistance().val() : 1.0;
      }

//---------------------------------------------------------
//   scaleNotesToLines
//    returns true if logical line = physical line
//---------------------------------------------------------

bool Staff::scaleNotesToLines() const
      {
      // TODO: make style option
      return !isDrumStaff();
      }

//---------------------------------------------------------
//   middleLine
//    returns logical line number of middle staff line
//---------------------------------------------------------

int Staff::middleLine() const
      {
      int line = lines() - 1;
      if (scaleNotesToLines())
            return line;
      else
            return line * lineDistance() / logicalLineDistance();
      //return isTabStaff() ? line : line * lineDistance() / logicalLineDistance();
      }

//---------------------------------------------------------
//   bottomLine
//    returns logical line number of bottom staff line
//---------------------------------------------------------

int Staff::bottomLine() const
      {
      int line = (lines() - 1) * 2;
      if (scaleNotesToLines())
            return line;
      else
            return line * lineDistance() / logicalLineDistance();
      //return isTabStaff() ? line : line * lineDistance() / logicalLineDistance();
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Staff::slashStyle() const
      {
      return _staffType.slashStyle();
      }

//---------------------------------------------------------
//   setSlashStyle
//---------------------------------------------------------

void Staff::setSlashStyle(bool val)
      {
      _staffType.setSlashStyle(val);
      }

//---------------------------------------------------------
//   linkTo
//---------------------------------------------------------

void Staff::linkTo(Staff* staff)
      {
      if (!_linkedStaves) {
            if (staff->linkedStaves()) {
                  _linkedStaves = staff->linkedStaves();
                  }
            else {
                  _linkedStaves = new LinkedStaves;
                  _linkedStaves->add(staff);
                  staff->setLinkedStaves(_linkedStaves);
                  }
            _linkedStaves->add(this);
            }
      else {
            _linkedStaves->add(staff);
            if (!staff->linkedStaves())
                  staff->_linkedStaves = _linkedStaves;
            }
      }

//---------------------------------------------------------
//   unlink
//---------------------------------------------------------

void Staff::unlink(Staff* staff)
      {
      if (!_linkedStaves)
            return;
      if (!_linkedStaves->staves().contains(staff))
            return;
      _linkedStaves->remove(staff);
      if (_linkedStaves->staves().size() <= 1) {
            delete _linkedStaves;
            _linkedStaves = 0;
            }
      staff->_linkedStaves = 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void LinkedStaves::add(Staff* staff)
      {
      if (!_staves.contains(staff))
            _staves.append(staff);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void LinkedStaves::remove(Staff* staff)
      {
      _staves.removeOne(staff);
      }

//---------------------------------------------------------
//   isLinked
///  return true if staff is different and
///  linked to this staff
//---------------------------------------------------------

bool Staff::isLinked(Staff* staff)
      {
      if (staff == this || !_linkedStaves)
            return false;

      for(Staff* s : _linkedStaves->staves()) {
            if(s == staff)
                  return true;
            }
      return false;
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
      if (!_linkedStaves)
            return true;
      QList<Staff*> s;
      QList<Staff*> ss;
      foreach(Staff* staff, _linkedStaves->staves()) {
            if (staff->score() == score()) {
                  s.append(staff);
                  if (!staff->isTabStaff())
                        ss.append(staff);
                  }
            }
      if (s.size() == 1) // the linked staves are in different scores
      	return s.front() == this;
      else // return a non tab linked staff in this score
      	return ss.front() == this;
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

void Staff::setStaffType(const StaffType* st)
      {
      if (_staffType == *st)
            return;
      int linesOld = lines();
      int linesNew = st->lines();
      _staffType = *st;

      if (linesNew != linesOld) {
            int sIdx = this->idx();
            if (sIdx < 0) {                     // staff does not belong to score (yet?)
                  if (linesNew == 1) {          // 1-line staves have special bar lines
                        _barLineFrom = BARLINE_SPAN_1LINESTAFF_FROM;
                        _barLineTo   = BARLINE_SPAN_1LINESTAFF_TO;
                  }
                  else {                        // set default barLineFrom/to (from first to last staff line)
                        _barLineFrom = 0;
                        _barLineTo   = (linesNew-1)*2;
                        }
                  }
            else                                // update barLineFrom/To in whole score context
                  score()->updateBarLineSpans(sIdx, linesOld, linesNew /*, true*/);
            }
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const InstrumentTemplate* t, const StaffType* staffType, int cidx)
      {
      // set staff-type-independent parameters
      if (cidx > MAX_STAVES) {
            setSmall(false);
            }
      else {
            setSmall(t->smallStaff[cidx]);
            setBracket(0, t->bracket[cidx]);
            setBracketSpan(0, t->bracketSpan[cidx]);
            setBarLineSpan(t->barlineSpan[cidx]);
            }
      const StaffType* pst = staffType ? staffType : t->staffTypePreset;
      if (!pst)
            pst = StaffType::getDefaultPreset(t->staffGroup);

      setStaffType(pst);
      setDefaultClefType(t->clefType(cidx));
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const Staff* s)
      {
      setStaffType(s->staffType());
      setDefaultClefType(s->defaultClefType());
      setSmall(s->small());
      _brackets          = s->_brackets;
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
      _userMag           = s->_userMag;
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
      setStaffType(staffType);
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
      if (_staffType.group() == StaffGroup::TAB)
            return false;
      else
            return _staffType.genKeysig();
      }

//---------------------------------------------------------
//   showLedgerLines
//---------------------------------------------------------

bool Staff::showLedgerLines()
      {
      return _staffType.showLedgerLines();
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
            if (s->type() == Element::Type::OTTAVA && s->staffIdx() == staffIdx) {
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
//      undoChangeProperty(P_ID::COLOR, val);
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
            int tick = i->first;
            _keys.erase(i++);
            kl2[tick + len] = kse;
            }
      _keys.insert(kl2.begin(), kl2.end());

      // check if there is a clef at the end of measure
      // before tick
      Clef* clef = 0;
      Measure* m = score()->tick2measure(tick);
      if (m && (m->tick() == tick) && (m->prevMeasure())) {
            m = m->prevMeasure();
            Segment* s = m->findSegment(Segment::Type::Clef, tick);
            if (s) {
                  int track = idx() * VOICES;
                  clef = static_cast<Clef*>(s->element(track));
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
      if (_linkedStaves)
            staffList = _linkedStaves->staves();
      else
            staffList.append(const_cast<Staff*>(this));
      return staffList;
      }

//---------------------------------------------------------
//   setBarLineTo
//---------------------------------------------------------

void Staff::setBarLineTo(int val)
      {
      _barLineTo = val;
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

QVariant Staff::getProperty(P_ID id) const
      {
      switch (id) {
            case P_ID::MAG:
                  return userMag();
            case P_ID::COLOR:
                  return color();
            case P_ID::SMALL:
                  return small();
            case P_ID::PLAYBACK_VOICE1:
                  return playbackVoice(0);
            case P_ID::PLAYBACK_VOICE2:
                  return playbackVoice(1);
            case P_ID::PLAYBACK_VOICE3:
                  return playbackVoice(2);
            case P_ID::PLAYBACK_VOICE4:
                  return playbackVoice(3);
            case P_ID::BARLINE_SPAN:
                  return barLineSpan();
            case P_ID::BARLINE_SPAN_FROM:
                  return barLineFrom();
            case P_ID::BARLINE_SPAN_TO:
                  return barLineTo();
            default:
                  qDebug("Staff::getProperty: unhandled id");
                  return QVariant();
            }
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Staff::setProperty(P_ID id, const QVariant& v)
      {
      switch (id) {
            case P_ID::MAG: {
                  double oldVal = mag();
                  setUserMag(v.toDouble());
                  scaleChanged(oldVal, mag());
                  }
                  break;
            case P_ID::COLOR:
                  setColor(v.value<QColor>());
                  break;
            case P_ID::SMALL: {
                  double oldVal = mag();
                  setSmall(v.toBool());
                  scaleChanged(oldVal, mag());
                  }
                  break;
            case P_ID::PLAYBACK_VOICE1:
                  setPlaybackVoice(0, v.toBool());
                  break;
            case P_ID::PLAYBACK_VOICE2:
                  setPlaybackVoice(1, v.toBool());
                  break;
            case P_ID::PLAYBACK_VOICE3:
                  setPlaybackVoice(2, v.toBool());
                  break;
            case P_ID::PLAYBACK_VOICE4:
                  setPlaybackVoice(3, v.toBool());
                  break;
            case P_ID::BARLINE_SPAN:
                  setBarLineSpan(v.toInt());
                  break;
            case P_ID::BARLINE_SPAN_FROM:
                  setBarLineFrom(v.toInt());
                  break;
            case P_ID::BARLINE_SPAN_TO:
                  setBarLineTo(v.toInt());
                  break;
            default:
                  qDebug("Staff::setProperty: unhandled id");
                  break;
            }
      score()->setLayoutAll();
      return true;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Staff::propertyDefault(P_ID id) const
      {
      switch (id) {
            case P_ID::MAG:
                  return 1.0;
            case P_ID::COLOR:
                  return QColor(Qt::black);
            case P_ID::SMALL:
                  return false;
            case P_ID::PLAYBACK_VOICE1:
            case P_ID::PLAYBACK_VOICE2:
            case P_ID::PLAYBACK_VOICE3:
            case P_ID::PLAYBACK_VOICE4:
                  return true;
            default:
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
      for (Segment* s = score()->firstSegment(); s; s = s->next1()) {
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
}

