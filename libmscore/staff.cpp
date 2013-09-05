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
#include "tablature.h"
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
      return NO_BRACKET;
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
      while (!_brackets.isEmpty() && (_brackets.last()._bracket == NO_BRACKET))
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
      if (!_brackets.isEmpty() && _brackets[0]._bracket == NO_BRACKET) {
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
            if (_brackets[i]._bracket == NO_BRACKET)
                  continue;
            int span = _brackets[i]._bracketSpan;
            if (span > (n - index)) {
                  span = n - index;
                  _brackets[i]._bracketSpan = span;
                  }
            }
      for (int i = 0; i < _brackets.size(); ++i) {
            if (_brackets[i]._bracket == NO_BRACKET)
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
      _keymap[0]      = KeySigEvent(0);                  // default to C major
      _staffType      = _score->staffType(STANDARD_STAFF_TYPE);
      _small          = false;
      _invisible      = false;
      _userDist       = .0;
      _barLineSpan    = 1;
      _barLineFrom    = 0;
      _barLineTo      = (lines()-1)*2;
      _updateKeymap   = true;
      _linkedStaves   = 0;
      setClef(0, ClefType::G);
      }

Staff::Staff(Score* s, Part* p, int rs)
      {
      _score          = s;
      _rstaff         = rs;
      _part           = p;
      _keymap[0]      = KeySigEvent(0);                  // default to C major
      _staffType      = _score->staffType(STANDARD_STAFF_TYPE);
      _small          = false;
      _invisible      = false;
      _userDist       = .0;
      _barLineSpan    = 1;
      _barLineFrom    = 0;
      _barLineTo      = (lines()-1)*2;
      _updateKeymap   = true;
      _linkedStaves   = 0;
      setClef(0, ClefType::G);
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
      ClefTypeList c = clefs.clef(tick);
      return score()->concertPitch() ? c._concertClef : c._transposingClef;
      }

ClefType Staff::clef(Segment* segment) const
      {
      return clef(segment->tick());
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
//   addClef
//---------------------------------------------------------

void Staff::addClef(Clef* clef)
      {
      int tick = clef->segment()->tick();
      if (tick == 0 || !clef->generated())
            clefs.setClef(tick, clef->clefTypeList());
      }

//---------------------------------------------------------
//   setClef
//---------------------------------------------------------

void Staff::setClef(int tick, const ClefTypeList& ctl)
      {
      clefs.setClef(tick, ctl);
      }

void Staff::setClef(int tick, ClefType ct)
      {
      setClef(tick, ClefTypeList(ct, ct));
      }

//---------------------------------------------------------
//   removeClef
//---------------------------------------------------------

void Staff::removeClef(Clef* clef)
      {
      if (clef->generated())
            return;
      int tick = clef->segment()->tick();
      if (tick == 0) {
            setClef(0, ClefType::G);
            return;
            }
      auto i = clefs.find(tick);
      if (i != clefs.end())
            clefs.erase(i);
      else
            qDebug("Staff::removeClef: Clef at %d not found", tick);
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
      return Groups::endings(m->timesig());
      }

//---------------------------------------------------------
//   addTimeSig
//---------------------------------------------------------

void Staff::addTimeSig(TimeSig* timesig)
      {
      timesigs[timesig->segment()->tick()] = timesig;
      }

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void Staff::removeTimeSig(TimeSig* timesig)
      {
      timesigs.erase(timesig->segment()->tick());
      }

//---------------------------------------------------------
//   Staff::key
//
//    locates the key sig currently in effect at tick
//---------------------------------------------------------

KeySigEvent Staff::key(int tick) const
      {
      return _keymap.key(tick);
      }

//---------------------------------------------------------
//   Staff::nextKeyTick
//
//    return the tick at which the key sig after tick is located
//    return 0, if no such a key sig
//---------------------------------------------------------

int Staff::nextKeyTick(int tick) const
      {
      return _keymap.nextKeyTick(tick);
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
      xml.tag("type", score()->staffTypeIdx(_staffType));
      if (small() && !xml.excerptmode)    // switch small staves to normal ones when extracting part
            xml.tag("small", small());
      if (invisible())
            xml.tag("invisible", invisible());
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
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "type") {
                  StaffType* st = score()->staffType(e.readInt());
                  if (st) {
                        _staffType = st;
                        // set default barLineTo according to staff type (1-line staff bar lines are special)
                        _barLineFrom = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : 0);
                        _barLineTo   = (lines() == 1 ? BARLINE_SPAN_1LINESTAFF_TO : (lines() - 1) * 2);
                        }
                  }
            else if (tag == "small")
                  setSmall(e.readInt());
            else if (tag == "invisible")
                  setInvisible(e.readInt());
            else if (tag == "keylist")
                  _keymap.read(e, _score);
            else if (tag == "bracket") {
                  BracketItem b;
                  b._bracket     = BracketType(e.intAttribute("type", -1));
                  b._bracketSpan = e.intAttribute("span", 0);
                  _brackets.append(b);
                  e.readNext();
                  }
            else if (tag == "barLineSpan") {
// WARNING: following statement assumes number of staff lines to be correctly set
                  // must read attributes before reading the main value
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
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   height
//---------------------------------------------------------

qreal Staff::height() const
      {
      return (lines()-1) * spatium() * _staffType->lineDistance().val();
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
      return _small ? score()->styleD(ST_smallStaffMag) : 1.0;
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void Staff::setKey(int tick, int st)
      {
      KeySigEvent ke;
      ke.setAccidentalType(st);

      setKey(tick, ke);
      }

void Staff::setKey(int tick, const KeySigEvent& st)
      {
      _keymap[tick] = st;
      }

//---------------------------------------------------------
//   removeKey
//---------------------------------------------------------

void Staff::removeKey(int tick)
      {
      _keymap.erase(tick);
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

int Staff::channel(int tick,  int voice) const
      {
      if (_channelList[voice].isEmpty())
            return 0;
      QMap<int, int>::const_iterator i = _channelList[voice].lowerBound(tick);
      if (i == _channelList[voice].begin())
            return _channelList[voice].begin().value();
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   lines
//---------------------------------------------------------

int Staff::lines() const
      {
      return _staffType->lines();
      }

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void Staff::setLines(int val)
      {
      if (val == lines())
            return;
      //
      // create new staff type
      //
      StaffType* st = _staffType->clone();
      if(part() && !partName().isEmpty())
            st->setName(partName());
      st->setLines(val);
      setStaffType(st);
      score()->addStaffType(st);
      }

//---------------------------------------------------------
//   line distance
//---------------------------------------------------------

qreal Staff::lineDistance() const
      {
      return _staffType->lineDistance().val();
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Staff::slashStyle() const
      {
      return _staffType->slashStyle();
      }

//---------------------------------------------------------
//   setSlashStyle
//---------------------------------------------------------

void Staff::setSlashStyle(bool val)
      {
      _staffType->setSlashStyle(val);
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
            }
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

void Staff::setStaffType(StaffType* st)
      {
      if (_staffType == st)
            return;
      int linesOld = lines();
      int linesNew = st->lines();
      _staffType = st;
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
      ClefType ct = clef(0);
      StaffGroup csg = ClefInfo::staffGroup(ct);

      if (_staffType->group() != csg) {
            switch(_staffType->group()) {
                  case TAB_STAFF_GROUP:        ct = ClefType(score()->styleI(ST_tabClef)); break;
                  case STANDARD_STAFF_GROUP:   ct = ClefType::G; break;      // TODO: use preferred clef for instrument
                  case PERCUSSION_STAFF_GROUP: ct = ClefType::PERC; break;
                  }
            clefs.setClef(0, ClefTypeList(ct, ct));
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
            clefs.setClef(0, t->clefTypes[0]);
            }
      else {
            setSmall(t->smallStaff[cidx]);
            clefs.setClef(0, t->clefTypes[cidx]);     // initial clef will be fixed to staff-type clef by setStaffType()
            setBracket(0, t->bracket[cidx]);
            setBracketSpan(0, t->bracketSpan[cidx]);
            setBarLineSpan(t->barlineSpan[cidx]);
            }

      // determine staff type and set number of lines accordingly
      // set lines AFTER setting the staff type, so if lines are different, the right staff type is cloned
      StaffType* st = 0;
      // get staff type if given or from instrument staff type, if not given
      // (if none, get default for staff group)
      const StaffType* presetStaffType = (staffType ? staffType : StaffType::preset(t->staffTypePreset) );
      if (!presetStaffType)
            presetStaffType = StaffType::getDefaultPreset(t->staffGroup, 0);

      // look for a staff type with same structure among staff types already defined in the score
      bool found = false;
      foreach (StaffType** scoreStaffType, score()->staffTypes()) {
            if ( (*scoreStaffType)->isSameStructure(*presetStaffType) ) {
                  st = *scoreStaffType;         // staff type found in score: use for instrument staff
                  found = true;
                  break;
                  }
            }
      // if staff type not found in score, use from preset (for staff and adding to score staff types)
      if (!found) {
            st = presetStaffType->clone();
            score()->addStaffType(st);
            }

      // use selected staff type
      setStaffType(st);
//      if (st->group() == PITCHED_STAFF)         // if PITCHED (in other staff groups num of lines is determined by style)
//            setLines(t->staffLines[cidx]);      // use number of lines from instr. template
      }

//---------------------------------------------------------
//   initFromStaffType
//---------------------------------------------------------

void Staff::initFromStaffType(const StaffType* staffType)
      {
      // get staff type if given (if none, get default preset for default staff group)
      const StaffType* presetStaffType = staffType;
      if (!presetStaffType)
            presetStaffType = StaffType::getDefaultPreset(STANDARD_STAFF_GROUP, 0);

      // look for a staff type with same structure among staff types already defined in the score
      StaffType* st = 0;
      foreach (StaffType** scoreStaffType, score()->staffTypes()) {
            if ( (*scoreStaffType)->isSameStructure(*presetStaffType) ) {
                  st = *scoreStaffType;         // staff type found in score: use for instrument staff
                  break;
                  }
            }
      // if staff type not found in score, use from preset (for staff and adding to score staff types)
      if (!st) {
            st = presetStaffType->clone();
            score()->addStaffType(st);
            }

      // use selected staff type
      setStaffType(st);
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
      switch(_staffType->group()) {
            case TAB_STAFF_GROUP:
                  return false;
            case STANDARD_STAFF_GROUP:
                  return static_cast<StaffTypePitched*>(_staffType)->genKeysig();
            case PERCUSSION_STAFF_GROUP:
                  return static_cast<StaffTypePercussion*>(_staffType)->genKeysig();
            default:
                  return true;
            }
      }

//---------------------------------------------------------
//   showLedgerLines
//---------------------------------------------------------

bool Staff::showLedgerLines()
      {
      switch(_staffType->group()) {
            case TAB_STAFF_GROUP:
                  return false;
            case STANDARD_STAFF_GROUP:
                  return static_cast<StaffTypePitched*>(_staffType)->showLedgerLines();
            case PERCUSSION_STAFF_GROUP:
                  return static_cast<StaffTypePercussion*>(_staffType)->showLedgerLines();
            default:
                  return true;
            }
      }

//---------------------------------------------------------
//   updateOttava
//---------------------------------------------------------

void Staff::updateOttava(Ottava* o)
      {
      pitchOffsets().setPitchOffset(o->tick(), o->pitchShift());
      pitchOffsets().setPitchOffset(o->tick2()+1, 0);
      }
}

