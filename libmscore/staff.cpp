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

namespace Ms {

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Staff::idx() const
      {
      return _score->staffIdx(this);
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
      while (!_brackets.isEmpty() && (_brackets.last()._bracket == BracketType::NO_BRACKET))
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
      if (!_brackets.isEmpty() && _brackets[0]._bracket == BracketType::NO_BRACKET) {
            _brackets[0] = b;
            }
      else {
            //
            // create new bracket level
            //
            foreach(Staff* s, _score->staves()) {
                  if (s == this)
                        s->_brackets.append(b);
                  else
                        s->_brackets.append(BracketItem());
                  }
            }
      }

//---------------------------------------------------------
//   cleanupBrackets
//---------------------------------------------------------

void Staff::cleanupBrackets()
      {
      int index = idx();
      int n = _score->nstaves();
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
      {
      _score          = s;
      _rstaff         = 0;
      _part           = 0;
      _barLineTo      = (lines()-1)*2;
      }

Staff::Staff(Score* s, Part* p, int rs)
      {
      _score          = s;
      _rstaff         = rs;
      _part           = p;
      _barLineTo      = (lines()-1)*2;
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
      if (_linkedStaves) {
            _linkedStaves->remove(this);
            if (_linkedStaves->isEmpty())
                  delete _linkedStaves;
            }
      }

//---------------------------------------------------------
//   Staff::clefTypeList
//---------------------------------------------------------

ClefTypeList Staff::clefTypeList(int tick) const
      {
      return clefs.clef(tick);
      }

//---------------------------------------------------------
//   Staff::clef
//---------------------------------------------------------

ClefType Staff::clef(int tick) const
      {
      ClefTypeList c = clefTypeList(tick);
      return score()->styleB(StyleIdx::concertPitch) ? c._concertClef : c._transposingClef;
      }

//---------------------------------------------------------
//   setInitialClef
//---------------------------------------------------------

void Staff::setInitialClef(ClefType ct)
      {
      clefs.setInitial(ClefTypeList(ct,ct));
      }

void Staff::setInitialClef(const ClefTypeList& ctl)
      {
      clefs.setInitial(ctl);
      }

//---------------------------------------------------------
//   initialClefTypeList
//---------------------------------------------------------

ClefTypeList Staff::initialClefTypeList() const
      {
      return clefs.initial();
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void Staff::setClef(Clef* clef)
      {
      int tick = clef->segment()->tick();
      for (Segment* s = clef->segment()->next(); s && s->tick() == tick; s = s->next()) {
            if (s->segmentType() == Segment::Type::Clef && s->element(clef->track())) {
                  // adding this clef has no effect on the clefs list
                  return;
                  }
            }
      clefs.setClef(clef->segment()->tick(), clef->clefTypeList());
      }

//---------------------------------------------------------
//   removeClef
//---------------------------------------------------------

void Staff::removeClef(Clef* clef)
      {
      int tick = clef->segment()->tick();

      for (Segment* s = clef->segment()->next(); s && s->tick() == tick; s = s->next()) {
            if (s->segmentType() == Segment::Type::Clef && s->element(clef->track())) {
                  // removal of this clef has no effect on the clefs list
                  return;
                  }
            }
      clefs.erase(clef->segment()->tick());
      for (Segment* s = clef->segment()->prev(); s && s->tick() == tick; s = s->prev()) {
            if (s->segmentType() == Segment::Type::Clef && s->element(clef->track())) {
                  // a previous clef at the same tick position gets valid
                  clefs.setClef(tick, static_cast<Clef*>(s->element(clef->track()))->clefTypeList());
                  break;
                  }
            }
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
      }

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void Staff::removeTimeSig(TimeSig* timesig)
      {
      if (timesig->segment()->segmentType() == Segment::Type::TimeSig)
            timesigs.erase(timesig->segment()->tick());
      }

//---------------------------------------------------------
//   Staff::key
//
//    locates the key sig currently in effect at tick
//---------------------------------------------------------

Key Staff::key(int tick) const
      {
      return _keys.key(tick);
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void Staff::setKey(int tick, Key k)
      {
      _keys.setKey(tick, k);
      }

//---------------------------------------------------------
//   removeKey
//---------------------------------------------------------

void Staff::removeKey(int tick)
      {
      _keys.erase(tick);
      }

//---------------------------------------------------------
//   prevkey
//---------------------------------------------------------

Key Staff::prevKey(int tick) const
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
      int idx = score()->staffIdx(this);
      xml.stag(QString("Staff id=\"%1\"").arg(idx + 1));
      if (linkedStaves()) {
            Score* s = score();
            if (s->parentScore())
                  s = s->parentScore();
            foreach(Staff* staff, linkedStaves()->staves()) {
                  if ((staff->score() == s) && (staff != this))
                        xml.tag("linkedTo", s->staffIdx(staff) + 1);
                  }
            }

      // for copy/paste we need to know the actual transposition
      if (xml.clipboardmode) {
            Interval v = part()->instr(0)->transpose();
            if (v.diatonic)
                  xml.tag("transposeDiatonic", v.diatonic);
            if (v.chromatic)
                  xml.tag("transposeChromatic", v.chromatic);
            }

      _staffType.write(xml);

      if (small() && !xml.excerptmode)    // switch small staves to normal ones when extracting part
            xml.tag("small", small());
      if (invisible())
            xml.tag("invisible", invisible());
      if (neverHide())
            xml.tag("neverHide", neverHide());

      foreach(const BracketItem& i, _brackets)
            xml.tagE("bracket type=\"%d\" span=\"%d\"", i._bracket, i._bracketSpan);

      // for economy and consistency, only output "from" and "to" attributes if different from default
      int defaultLineFrom = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
      int defaultLineTo;
      if (_barLineSpan == 0)                    // if no bar line at all
            defaultLineTo = _barLineTo;         // whatever the current spanTo is, use as default
      else {                                    // if some bar line, default is the default for span target staff
            int targetStaffIdx = idx + _barLineSpan - 1;
            if (targetStaffIdx >= score()->nstaves()) {
                  qFatal("bad _barLineSpan %d for staff %d (nstaves %d)",
                     _barLineSpan, idx, score()->nstaves());
                  }
            int targetStaffLines = score()->staff(targetStaffIdx)->lines();
            defaultLineTo = (targetStaffLines == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (targetStaffLines-1) * 2);
            }
      if (_barLineSpan != 1 || _barLineFrom != defaultLineFrom || _barLineTo != defaultLineTo) {
            if(_barLineFrom != defaultLineFrom || _barLineTo != defaultLineTo)
                  xml.tag(QString("barLineSpan from=\"%1\" to=\"%2\"").arg(_barLineFrom).arg(_barLineTo), _barLineSpan);
            else
                  xml.tag("barLineSpan", _barLineSpan);
            }
      if (_userDist != 0.0)
            xml.tag("distOffset", _userDist / spatium());
      if (color() != Qt::black)
            xml.tag("color", color());
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
                  qDebug("Staff::read staffTypeIdx %d", staffTypeIdx);
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
            else if (tag == "small")
                  setSmall(e.readInt());
            else if (tag == "invisible")
                  setInvisible(e.readInt());
            else if (tag == "neverHide")
                  setNeverHide(e.readInt());
            else if (tag == "keylist")
                  _keys.read(e, _score);
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
                  _barLineFrom = e.attribute("from", QString::number(defaultSpan)).toInt();
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
                  _userDist = e.readDouble() * spatium();
            else if (tag == "linkedTo") {
                  int v = e.readInt() - 1;
                  //
                  // if this is an excerpt, link staff to parentScore()
                  //
                  if (score()->parentScore()) {
                        Staff* st = score()->parentScore()->staff(v);
                        if (st)
                              linkTo(st);
                        else {
                              qDebug("staff %d not found in parent", v);
                              }
                        }
                  else {
                        int idx = score()->staffIdx(this);
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
      return _score->spatium() * mag();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Staff::mag() const
      {
      return _small ? score()->styleD(StyleIdx::smallStaffMag) : 1.0;
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

int Staff::channel(int tick,  int voice) const
      {
      if (_channelList[voice].isEmpty())
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
//   line distance
//---------------------------------------------------------

qreal Staff::lineDistance() const
      {
      return _staffType.lineDistance().val();
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
      Q_ASSERT(_linkedStaves->staves().contains(staff));
      _linkedStaves->remove(staff);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void LinkedStaves::add(Staff* staff)
      {
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
///   the one who is played back
//---------------------------------------------------------

bool Staff::primaryStaff() const
      {
      QList<Staff*> s;
      if (!_linkedStaves)
            return true;
      foreach(Staff* staff, _linkedStaves->staves()) {
            if (staff->score() == score())
                  s.append(staff);
            }
      return s.front() == this;
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
            int sIdx = score()->staffIdx(this);
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

      //
      //    check for right clef-type and fix
      //    if necessary
      //
      ClefType ct    = clefs.initial()._concertClef;
      StaffGroup csg = ClefInfo::staffGroup(ct);

      if (_staffType.group() != csg) {
            switch(_staffType.group()) {
                  case StaffGroup::TAB:
                        ct = ClefType(score()->styleI(StyleIdx::tabClef));
                        break;
                  case StaffGroup::STANDARD:
                        ct = ClefType::G;       // TODO: use preferred clef for instrument
                        break;
                  case StaffGroup::PERCUSSION:
                        ct = ClefType::PERC;
                        break;
                  }
            setInitialClef(ct);
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
//      if (pst->group() == ArticulationShowIn::PITCHED_STAFF)         // if PITCHED (in other staff groups num of lines is determined by style)
//            setLines(t->staffLines[cidx]);      // use number of lines from instr. template
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
      if (_staffType.group() == StaffGroup::TAB)
            return false;
      else
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
//      score()->undoChangeProperty(this, P_ID::COLOR, val);
      }

//---------------------------------------------------------
//   insertTime
//---------------------------------------------------------

void Staff::insertTime(int tick, int len)
      {
      KeyList kl2;
      for (auto i = _keys.upper_bound(tick); i != _keys.end();) {
            Key kse = i->second;
            int k   = i->first;
            _keys.erase(i++);
            kl2[k + len] = kse;
            }
      _keys.insert(kl2.begin(), kl2.end());

      ClefList cl2;
      for (auto i = clefs.upper_bound(tick); i != clefs.end();) {
            ClefTypeList ctl = i->second;
            int key = i->first;
            clefs.erase(i++);
            cl2.setClef(key + len, ctl);
            }
      clefs.insert(cl2.begin(), cl2.end());
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

}

