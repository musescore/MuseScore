//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scnote.cpp 5427 2012-03-07 12:41:34Z wschweer $
//
//  Copyright (C) 2009-2010 Werner Schweer and others
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

#include "musescore.h"
#include "scnote.h"
#include "libmscore/note.h"
#include "libmscore/utils.h"
#include "libmscore/undo.h"
#include "script.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/page.h"

Q_DECLARE_METATYPE(Note);
Q_DECLARE_METATYPE(Note*);
Q_DECLARE_METATYPE(Score*);

static const char* const function_names_note[] = {
      "name", "pitch", "tuning", "color", "visible", "tpc", "tied", "userAccidental",
      "boundingRect", "pos", "noteHead", "velocity"
      };
static const int function_lengths_note[] = {
      0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 1
      };

static const QScriptValue::PropertyFlags flags_note[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
	  QScriptValue::SkipInEnumeration,
	  QScriptValue::SkipInEnumeration,
	  QScriptValue::SkipInEnumeration
      };

ScriptInterface noteInterface = {
      sizeof(function_names_note) / sizeof(*function_names_note),
      function_names_note,
      function_lengths_note,
      flags_note
      };

//---------------------------------------------------------
//   prototype_note_call
//---------------------------------------------------------

static QScriptValue prototype_Note_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Note* note = qscriptvalue_cast<Note*>(context->thisObject());
      if (!note) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Note.%0(): this object is not a Note")
               .arg(function_names_note[_id]));
            }

      int argc = context->argumentCount();
      switch(_id) {
            case 0:   //name
                  {
                  bool germanNames = (argc == 0) ? context->argument(1).toBool() : false;
                  if (argc == 0 || argc == 1)
                        return qScriptValueFromValue(context->engine(), tpc2name(note->tpc(), germanNames));
                  }
                  break;
            case 1:   //pitch
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), note->pitch());
                  else if (argc == 1) {
                        int pitch = context->argument(0).toInt32();
                        if (pitch < 0 || pitch > 127)
                              break;
                        Score* score = note->score();
                        if (score) {
                              //TODO fix this for trunk
/*                            Note* tmp = note->clone();
                              tmp->setPitch(pitch);
                              tmp->setTpcFromPitch();
                              score->undoChangePitch(note, pitch, tmp->tpc(), note->userAccidental());
                              delete tmp;
*/
                              note->setPitch(pitch);
                              note->setTpcFromPitch();
                              }
                        else {
                              note->setPitch(pitch);
                              note->setTpcFromPitch();
                              }
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 2:    // tuning
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), note->tuning());
                  else if (argc == 1) {
                        double tuning = context->argument(0).toNumber();
                        Score* score = note->score();
                        if (score) {
                              score->undoChangeTuning(note, tuning);
                              }
                        else{
                              note->setTuning(tuning);
                              }
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 3:   // color
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), note->color());
                  else if (argc == 1) {
                        QColor c = qscriptvalue_cast<QColor>(context->argument(0));
                        Score* score = note->score();
                        if (score)
                              score->undo(new ChangeProperty(note, P_COLOR, c));
                        else
                              note->setColor(c);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 4:   //visible
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), note->visible());
                  else if (argc == 1) {
                        bool v = context->argument(0).toInt32();
                        note->setVisible(v);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 5:   //tpc
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), note->tpc());
                  else if (argc == 1) {
                        int v = context->argument(0).toInt32();
                        if (v < -1 || v > 33)
                              break;
                        note->setTpc(v);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 6:   //tied
                  if (argc == 0) {
                        int tiemode = 0;
                        if (note->tieFor())
                              tiemode |= 1;
                        if (note->tieBack())
                              tiemode |= 2;
                        return qScriptValueFromValue(context->engine(), tiemode);
                        }
                  break;
            case 7:   //userAccidental

                  if (argc == 0)
                        // return qScriptValueFromValue(context->engine(), int(note->userAccidental()));
                        return qScriptValueFromValue(context->engine(), 0);
                  else if (argc == 1) {
                        // int v = context->argument(0).toInt32();
                        // TODO: does not work:       note->setAccidentalType(AccidentalType(v));
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 8:     // "boundingRect"
  				        if (context->argumentCount() == 0)
  					           return qScriptValueFromValue(context->engine(), note->bbox());
  				        break;
          	case 9:     // "pos"
        				  if (context->argumentCount() == 0){
              				  Page* page = (Page*)note->parent()->parent()->parent()->parent()->parent();
              				  QPointF pos(note->pagePos().x() - page->pagePos().x(),  note->pagePos().y());
              				  return qScriptValueFromValue(context->engine(), pos);
                        }
          			  break;
      			case 10:     // "noteHead"
      				  if (context->argumentCount() == 0)
      					  return qScriptValueFromValue(context->engine(), note->noteHead());
      					else if (context->argumentCount() == 1) {
            				  int v = context->argument(0).toInt32();
                      if(v < HEAD_GROUPS) {
                            Score* score = note->score();
                            if (score)
                                  score->undoChangeProperty(note, P_HEAD_GROUP, v);
                            else
                                  note->setHeadGroup(NoteHeadGroup(v));
                            }
                      return context->engine()->undefinedValue();
                      }
      				  break;
            case 11:     // "velocity"
      				  if (context->argumentCount() == 0)
      					  return qScriptValueFromValue(context->engine(), note->veloOffset());
      					else if (context->argumentCount() == 1) {
            				  int v = context->argument(0).toInt32();
            				  Score* score = note->score();
                      if (!score)
                           return context->engine()->undefinedValue();
                      if(v < 0) {
                            if (note->veloType() != OFFSET_VAL) {
                                  score->undo(new ChangeNoteProperties(note,
                                      OFFSET_VAL, note->veloOffset(),
                                      note->onTimeUserOffset(), note->offTimeUserOffset()));
                                      //score->updateVelo();
                                  }
                           }
                      else if (v < 127) {
                            score->undo(new ChangeNoteProperties(note,
                                 USER_VAL, v,
                                 note->onTimeUserOffset(), note->offTimeUserOffset()));
                            }
                      return context->engine()->undefinedValue();
                      }
      				  break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Note.%0(): bad argument count or value")
         .arg(function_names_note[_id]));
      }

//---------------------------------------------------------
//   static_Note_call
//---------------------------------------------------------

static QScriptValue static_Note_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Note(): Did you forget to construct with 'new'?"));
      Note* note = 0;
      if (context->argumentCount() == 0)
            note = new Note(0);
      else if (context->argumentCount() == 1) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            note   = new Note(score);
            }
      if (note)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(note));
      return context->throwError(QString::fromLatin1("Note(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_Note_class
//---------------------------------------------------------

QScriptValue create_Note_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &noteInterface;

      engine->setDefaultPrototype(qMetaTypeId<Note*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Note*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Note_call, function_lengths_note[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Note*>(), proto);
      return engine->newFunction(static_Note_call, proto, 1);
      }



