//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scchord.cpp 4388 2011-06-18 13:17:58Z wschweer $
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
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "script.h"
#include "libmscore/harmony.h"
#include "libmscore/measure.h"
#include "libmscore/lyrics.h"

Q_DECLARE_METATYPE(Chord);
Q_DECLARE_METATYPE(Chord*);
Q_DECLARE_METATYPE(Harmony*);
Q_DECLARE_METATYPE(Score*);
Q_DECLARE_METATYPE(Note*);

static const char* const function_names_chord[] = {
      "tickLen", "addHarmony", "topNote", "addNote", "removeNote", "notes", "note", "type", "lyrics", "noStem", "small"
      };
static const int function_lengths_chord[] = {
      1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1
      };
static const QScriptValue::PropertyFlags flags_chord[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::ReadOnly,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::ReadOnly
      };

ScriptInterface chordInterface = {
      sizeof(function_names_chord) / sizeof(*function_names_chord),
      function_names_chord,
      function_lengths_chord,
      flags_chord
      };

//---------------------------------------------------------
//   prototype_Chord_call
//---------------------------------------------------------

static QScriptValue prototype_Chord_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Chord* chord = qscriptvalue_cast<Chord*>(context->thisObject());
      if (!chord) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Chord.%0(): this object is not a Chord")
               .arg(function_names_chord[_id]));
            }
      switch(_id) {
            case 0:
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), chord->actualTicks());
                  else if (context->argumentCount() == 1) {
                        int ticks = context->argument(0).toInt32();
                        if (ticks < 1)
                              break;
                        chord->setDurationType(ticks);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 1:
                  if (context->argumentCount() == 1) {
                        Harmony* h = qscriptvalue_cast<Harmony*>(context->argument(0));
                        if (!h)
                              break;
                        h->setParent(chord->measure());
//TODO1                        h->setTick(chord->tick());
                        Score* score = chord->score();
                        if (score) {
                              h->setScore(score);
                              score->undoAddElement(h);
                              }
                        else
                              chord->measure()->add(h);
                        h->render();
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 2:     // "topNote",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), chord->upNote());
                  break;
            case 3:     // "addNote",
                  if (context->argumentCount() == 1) {
                        Note* note = qscriptvalue_cast<Note*>(context->argument(0));
                        if (!note)
                              break;
                        note->setParent(chord);
                        Score* score = chord->score();
                        if (score) {
                              note->setScore(score);
                              chord->score()->undoAddElement(note);
                              }
                        else
                              chord->add(note);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 4:     // "removeNote",
                  if (context->argumentCount() == 1) {
                        int idx = context->argument(0).toInt32();
                        if (idx < 0 || idx >= int(chord->notes().size()))
                              return context->throwError(QScriptContext::TypeError,
                                 QString::fromLatin1("Chord.%0(): note index out of range")
                                 .arg(function_names_chord[_id]));
                        Score* score = chord->score();
                        if (score)
                              score->undoRemoveElement(chord->notes()[idx]);
                        else
                              chord->notes().removeAt(idx);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 5:     // "notes",
                  if (context->argumentCount() == 0) {
                        return qScriptValueFromValue(context->engine(), chord->notes().size());
                        }
                  break;
            case 6:     // "note"
                  if (context->argumentCount() == 1) {
                        int idx = context->argument(0).toInt32();
                        if (idx < 0 || idx >= int(chord->notes().size())) {
                              return context->throwError(QScriptContext::TypeError,
                                 QString::fromLatin1("Chord.%0(): note index out of range")
                                 .arg(function_names_chord[_id]));
                              }
                        return qScriptValueFromValue(context->engine(), chord->notes()[idx]);
                        }
                  break;
            case 7:     // "type"
                  return qScriptValueFromValue(context->engine(), int(chord->noteType()));
            case 8:     // "lyric"
                  {
                  //TODO adapt to new lyric code
                  /*QStringList ll;
                  LyricsList * lyrlist = chord->segment()->lyricsList(0);
	                for (ciLyrics lix = lyrlist->begin(); lix != lyrlist->end(); ++lix)
                      ll.append((*lix)->getText());
                  return qScriptValueFromValue(context->engine(), ll);*/
                  return context->engine()->undefinedValue();
                  }
            case 9:     //noStem
                 {
                 if (context->argumentCount() == 0) {
                      return qScriptValueFromValue(context->engine(), chord->noStem());
                      }
                 else if (context->argumentCount() == 1) {
                      bool noStem = context->argument(0).toBool();
                      chord->score()->undoChangeChordNoStem(chord, noStem);
                      }
                return context->engine()->undefinedValue();
                }
            case 10:     //small
                 {
                 if (context->argumentCount() == 0) {
                      return qScriptValueFromValue(context->engine(), chord->small());
                      }
                 else if (context->argumentCount() == 1) {
                      bool small = context->argument(0).toBool();
                      chord->score()->undoChangeChordRestSize(chord, small);
                      }
                return context->engine()->undefinedValue();
                }
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Chord.%0(): bad argument count or value")
         .arg(function_names_chord[_id]));
      }

//---------------------------------------------------------
//   static_Chord_call
//---------------------------------------------------------

static QScriptValue static_Chord_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Chord(): Did you forget to construct with 'new'?"));
      Chord* chord = 0;
      if (context->argumentCount() == 0)
            chord = new Chord(0);
      else if (context->argumentCount() == 1) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            chord   = new Chord(score);
            }
      if (chord)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(chord));
      return context->throwError(QString::fromLatin1("Chord(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_Chord_class
//---------------------------------------------------------

QScriptValue create_Chord_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &chordInterface;

      engine->setDefaultPrototype(qMetaTypeId<Chord*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Chord*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Chord_call, function_lengths_chord[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Chord*>(), proto);
      return engine->newFunction(static_Chord_call, proto, 1);
      }
