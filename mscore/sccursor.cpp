//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: sccursor.cpp 5427 2012-03-07 12:41:34Z wschweer $
//
//  Copyright (C) 2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "libmscore/chordrest.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/note.h"
#include "libmscore/stafftext.h"
#include "libmscore/text.h"
#include "libmscore/measure.h"
#include "libmscore/repeatlist.h"
#include "libmscore/page.h"
#include "script.h"
#include "libmscore/system.h"
#include "sccursor.h"
#include "libmscore/segment.h"

//---------------------------------------------------------
//   SCursor
//---------------------------------------------------------

SCursor::SCursor(Score* s)
      {
      _score    = s;
      _staffIdx = 0;
      _voice    = 0;
      _segment  = 0;
      _expandRepeat = false;
      _curRepeatSegment = 0;
      _curRepeatSegmentIndex = 0;
      }

SCursor::SCursor(Score* s, bool expandRepeat)
      {
      _score    = s;
      _staffIdx = 0;
      _voice    = 0;
      _segment  = 0;
      _expandRepeat = expandRepeat;
      _curRepeatSegment = 0;
      _curRepeatSegmentIndex = 0;
      _score->updateRepeatList(expandRepeat);
      }

//---------------------------------------------------------
//   cr
//---------------------------------------------------------

ChordRest* SCursor::cr() const
      {
      if (_segment) {
            int track = _staffIdx * VOICES + _voice;
            Element* e = _segment->element(track);
            if (e && (e->isChordRest() || e->type() == REPEAT_MEASURE))
                  return static_cast<ChordRest*>(e);
            }
      return 0;
      }

//---------------------------------------------------------
//   rewind
//---------------------------------------------------------

void SCursor::rewind(int type)
      {
      if (type == 0) {
            _segment   = 0;
            Measure* m = _score->firstMeasure();
            if (_expandRepeat && !_score->repeatList()->isEmpty()){
                  _curRepeatSegment = _score->repeatList()->first();
                  _curRepeatSegmentIndex = 0;
                  }
            if (m) {
                  _segment = m->first();
                  if (_staffIdx >= 0) {
                        int track = _staffIdx * VOICES + _voice;
                        while (_segment && ((_segment->subtype() != SegChordRest) || (_segment->element(track) == 0)))
                              _segment = _segment->next1();
                        }
                  }
            }
      else if (type == 1) {
            _segment  = _score->selection().startSegment();
            _staffIdx = _score->selection().staffStart();
            _voice    = 0;
            }
      else if (type == 2) {
            _segment  = _score->selection().endSegment();
            _staffIdx = _score->selection().staffEnd();
            _voice    = 0;
            }
      }

Q_DECLARE_METATYPE(SCursor);
Q_DECLARE_METATYPE(SCursor*);
Q_DECLARE_METATYPE(Chord*);
Q_DECLARE_METATYPE(Rest*);
Q_DECLARE_METATYPE(Score*);
Q_DECLARE_METATYPE(Measure*);
Q_DECLARE_METATYPE(Text*);

static const char* const function_names_cursor[] = {
      "rewind", "eos", "chord", "rest", "measure", "next", "nextMeasure", "putStaffText",  "isChord", "isRest",
      "add", "tick", "time", "staff", "voice", "pageNumber", "pos", "goToSelectionStart", "goToSelectionEnd",
      };
static const int function_lengths_cursor[] = {
      0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0
      };
static const QScriptValue::PropertyFlags flags_cursor[] = {
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration
      };

ScriptInterface cursorInterface = {
      sizeof(function_names_cursor) / sizeof(*function_names_cursor),
      function_names_cursor,
      function_lengths_cursor,
      flags_cursor
      };

//---------------------------------------------------------
//   prototype_Cursor_call
//---------------------------------------------------------

static QScriptValue prototype_Cursor_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      SCursor* cursor = qscriptvalue_cast<SCursor*>(context->thisObject());
      if (!cursor) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Cursor.%0(): this object is not a Cursor")
               .arg(function_names_cursor[_id]));
            }
      switch(_id) {
            case 0:     // "rewind",
                  if (context->argumentCount() == 0) {
                        cursor->rewind(0);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 1:     // "eos",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), cursor->segment() == 0);
                  break;
            case 2:     // "chord",
                  if (context->argumentCount() == 0) {
                        ChordRest* cr = cursor->cr();
                        if (cr->type() != CHORD)
                              cr = 0;
                        return qScriptValueFromValue(context->engine(), static_cast<Chord*>(cr));
                        }
                  break;
            case 3:     // "rest",
                  if (context->argumentCount() == 0) {
                        ChordRest* cr = cursor->cr();
                        if (cr->type() != REST)
                              cr = 0;
                        return qScriptValueFromValue(context->engine(), static_cast<Rest*>(cr));
                        }
                  break;
            case 4:     // "measure",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), cursor->cr()->measure());
                  break;
            case 5:     // "next",
                  if (context->argumentCount() == 0) {
                        cursor->next();
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 6:     // "nextMeasure",
                  if (context->argumentCount() == 0) {
                        cursor->nextMeasure();
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 7:     // "putStaffText",
                  if (context->argumentCount() == 1) {
                        Text* t = qscriptvalue_cast<Text*>(context->argument(0));
                        if (t) {
                              cursor->putStaffText(t);
                              return context->engine()->undefinedValue();
                              }
                        }
                  break;
            case 8:     // "isChord",
                  if (context->argumentCount() == 0) {
                        ChordRest* cr = cursor->cr();
                        bool val = cr ? cr->type() == CHORD : false;
                        return qScriptValueFromValue(context->engine(), val);
                        }
                  break;
            case 9:     // "isRest",
                  if (context->argumentCount() == 0) {
                        ChordRest* cr = cursor->cr();
                        bool val = cr ? cr->type() == REST : false;
                        return qScriptValueFromValue(context->engine(), val);
                        }
                  break;
            case 10:    // "add",
                  if (context->argumentCount() == 1) {
                        ChordRest* cr = qscriptvalue_cast<Chord*>(context->argument(0));
                        if (!cr)
                              cr = qscriptvalue_cast<Rest*>(context->argument(0));
                        if (cr) {
                              cursor->add(cr);
                              return context->engine()->undefinedValue();
                              }
                        }
                  break;
            case 11:    // "tick",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), cursor->tick());
                  break;
            case 12:    // "time"
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), cursor->time());
                  break;
            case 13:    // "staff"
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), cursor->staffIdx());
                  else if (context->argumentCount() == 1) {
                        int val = context->argument(0).toInt32();
                        if (val < 0)
                              val = 0;
                        else if (val >= cursor->score()->nstaves())
                              val = cursor->score()->nstaves() - 1;
                        cursor->setStaffIdx(val);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 14:    // "voice"
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), cursor->voice());
                  else if (context->argumentCount() == 1) {
                        int val = context->argument(0).toInt32();
                        if (val < 0)
                              val = 0;
                        else if (val >= VOICES)
                              val = VOICES - 1;
                        cursor->setVoice(val);
                        return context->engine()->undefinedValue();
                        }
                  break;
             case 15:    // "pageNumber",
                  if (context->argumentCount() == 0)
                        if(cursor->segment())
                            return qScriptValueFromValue(context->engine(), cursor->segment()->measure()->system()->page()->no());
                  break;
             case 16:    // "pos"
                  if (context->argumentCount() == 0){
                        if(cursor->segment()){
                              Page* page = (Page*)cursor->segment()->measure()->parent()->parent();
                              QPointF pos(cursor->segment()->pagePos().x() - page->pagePos().x(),  cursor->segment()->pagePos().y());
                              return qScriptValueFromValue(context->engine(), pos);
                            }
                        }
                  break;
             case 17:    // "goToSelectionStart"
                  if (context->argumentCount() == 0) {
                        cursor->rewind(1);
                        return context->engine()->undefinedValue();
                        }
                  break;
             case 18:    // "goToSelectionEnd"
                  if (context->argumentCount() == 0) {
                        cursor->rewind(2);
                        return context->engine()->undefinedValue();
                        }
                  break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Cursor.%0(): bad argument count or value")
         .arg(function_names_cursor[_id]));
      }

//---------------------------------------------------------
//   static_Cursor_call
//---------------------------------------------------------

static QScriptValue static_Cursor_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Cursor(): Did you forget to construct with 'new'?"));
      SCursor* cursor = 0;
      if (context->argumentCount() == 0)
            cursor = new SCursor(0);
      else if (context->argumentCount() == 1) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            cursor   = new SCursor(score);
            }
      else if (context->argumentCount() == 2) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            bool expandRepeat = context->argument(1).toBool();
            cursor   = new SCursor(score, expandRepeat);
            }
      if (cursor)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(cursor));
      return context->throwError(QString::fromLatin1("Cursor(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_Cursor_class
//---------------------------------------------------------

QScriptValue create_Cursor_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &cursorInterface;

      engine->setDefaultPrototype(qMetaTypeId<SCursor*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((SCursor*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Cursor_call, function_lengths_cursor[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<SCursor*>(), proto);
      return engine->newFunction(static_Cursor_call, proto, 1);
      }

//---------------------------------------------------------
//   next
//    go to next segment
//    return false if end of score is reached
//---------------------------------------------------------

bool SCursor::next()
      {
      if (!_segment)
            return false;
      Segment* seg = _segment;
      seg = seg->next1();
      RepeatSegment* rs = repeatSegment();
      if (rs && expandRepeat()){
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            if ((seg  && (seg->tick() >= endTick) ) || (!seg) ){
                  int rsIdx = repeatSegmentIndex();
                  rsIdx ++;
                  if (rsIdx < score()->repeatList()->size()){ //there is a next repeat segment
                        rs = score()->repeatList()->at(rsIdx);
                        setRepeatSegment(rs);
                        setRepeatSegmentIndex(rsIdx);
                        Measure* m = score()->tick2measure(rs->tick);
                        seg = m ? m->first() : 0;
                        }
                  else
                        seg = 0;
                  }
            }

      if (_staffIdx >= 0) {
            //int track = _staffIdx * VOICES + _voice;
            while (seg && (!(seg->subtype() & (SegChordRest | SegGrace)) /*|| !seg->element(track)*/)) {
                  seg = seg->next1();
                  }
            }
      _segment = seg;
      return seg != 0;
      }

//---------------------------------------------------------
//   nextMeasure
//    go to first segment of next measure
//    return false if end of score is reached
//---------------------------------------------------------

bool SCursor::nextMeasure()
      {
      Measure* m = segment()->measure();
      m = m->nextMeasure();
      RepeatSegment* rs = repeatSegment();
      if (rs && expandRepeat()){
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            if ((m  && (m->tick() + m->ticks() > endTick) ) || (!m) ){
                  int rsIdx = repeatSegmentIndex();
                  rsIdx ++;
                  if (rsIdx < score()->repeatList()->size()) {       //there is a next repeat segment
                        rs = score()->repeatList()->at(rsIdx);
                        setRepeatSegment(rs);
                        setRepeatSegmentIndex(rsIdx);
                        m = score()->tick2measure(rs->tick);
                        }
                  else{
                        m = 0;
                        }
                  }
            }

      if (m == 0) {
            setSegment(0);
            return false;
            }
      Segment* seg = m->first();
      if (_staffIdx >= 0) {
            int track = _staffIdx * VOICES + _voice;
            while (seg && ((seg->subtype() != SegChordRest) || (seg->element(track) == 0)))
                  seg = seg->next1();
            }
      _segment = seg;
      return seg != 0;
      }

//---------------------------------------------------------
//   putStaffText
//---------------------------------------------------------

void SCursor::putStaffText(Text* s)
      {
      if (!cr() || !s)
            return;
      s->setTrack(cr()->track());
      s->setTextStyleType(TEXT_STYLE_STAFF);
      s->setParent(cr()->measure());
      s->score()->undoAddElement(s);
      s->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SCursor::add(ChordRest* c)
      {
      ChordRest* chordRest = cr();
      int track       = _staffIdx * VOICES + _voice;

      if (!chordRest) {
            if(_voice > 0) { //create rests
                int t = tick();
                //trick : go to the start if we don't have segment nor chord.
                if(t == _score->lastMeasure()->tick() + _score->lastMeasure()->ticks())
                      t = 0;
                Measure* measure = score()->tick2measure(t);
                SegmentType st = SegChordRest;
                Segment* seg = measure->findSegment(st, t);
                if (seg == 0) {
                      seg = new Segment(measure, st, t);
                      score()->undoAddElement(seg);
                      }
                chordRest = score()->addRest(seg, track, TDuration(TDuration::V_MEASURE), 0);
                }
            if (!chordRest) {
                  qDebug("SCursor::add: no cr\n");
                  return;
                  }
            }
      int tick = chordRest->tick();
      Fraction len(c->durationType().fraction());

      Fraction gap    = score()->makeGap(chordRest->segment(), chordRest->track(),
         len, chordRest->tuplet());
      if (gap < len) {
            qDebug("cannot make gap\n");
            return;
            }
      Measure* measure = score()->tick2measure(tick);
      SegmentType st = SegChordRest;
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = new Segment(measure, st, tick);
            score()->undoAddElement(seg);
            }
      c->setScore(score());
      if (c->type() == CHORD) {
            foreach(Note* n, static_cast<Chord*>(c)->notes())
                  n->setScore(score());
            }

      setSegment(seg);
      c->setParent(seg);
      c->setTrack(track);
      score()->undoAddElement(c);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int SCursor::tick()
      {
      int offset = 0;
      RepeatSegment* rs = repeatSegment();
      if (rs && expandRepeat())
            offset = rs->utick - rs->tick;
      if (cr())
          return cr()->tick() + offset;
      else if (segment())
          return segment()->tick() + offset;
      else
          return _score->lastMeasure()->tick() + _score->lastMeasure()->ticks() + offset;  // end of score
      }

//---------------------------------------------------------
//   time
//---------------------------------------------------------

double SCursor::time()
      {
      return score()->utick2utime(tick()) * 1000;
      }
