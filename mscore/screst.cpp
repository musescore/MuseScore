//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/chordrest.h"
#include "libmscore/harmony.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "script.h"

Q_DECLARE_METATYPE(Rest);
Q_DECLARE_METATYPE(Rest*);
Q_DECLARE_METATYPE(Harmony*);
Q_DECLARE_METATYPE(Score*);

static const char* const function_names_rest[] = {
      "tickLen", "addHarmony", "small"
      };
static const int function_lengths_rest[] = {
      1, 1, 1
      };
static const QScriptValue::PropertyFlags flags_rest[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter
      };

ScriptInterface restInterface = {
      sizeof(function_names_rest) / sizeof(*function_names_rest),
      function_names_rest,
      function_lengths_rest,
      flags_rest
      };

//---------------------------------------------------------
//   prototype_Rest_call
//---------------------------------------------------------

static QScriptValue prototype_Rest_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Rest* rest = qscriptvalue_cast<Rest*>(context->thisObject());
      if (!rest) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Rest.%0(): this object is not a Rest")
               .arg(function_names_rest[_id]));
            }
      switch(_id) {
            case 0:
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), rest->actualTicks());
                  else if (context->argumentCount() == 1) {
                        int ticks = context->argument(0).toInt32();
                        if (ticks < 1)
                              break;
                        rest->setDurationType(ticks);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 1:
                  if (context->argumentCount() == 1) {
                        Harmony* h = qscriptvalue_cast<Harmony*>(context->argument(0));
                        if (!h)
                              break;
                        h->setParent(rest->measure());
//TODO1                        h->setTick(rest->tick());
                        Score* score = rest->score();
                        if (score) {
                              h->setScore(score);
                              score->undoAddElement(h);
                              }
                        else
                              rest->measure()->add(h);
                        h->render();
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 2:     //small
                 {
                 if (context->argumentCount() == 0) {
                      return qScriptValueFromValue(context->engine(), rest->small());
                      }
                 else if (context->argumentCount() == 1) {
                      bool small = context->argument(0).toBool();
                      rest->score()->undoChangeChordRestSize(rest, small);
                      }
                return context->engine()->undefinedValue();
                }
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Note.%0(): bad argument count or value")
         .arg(function_names_rest[_id]));
      }

//---------------------------------------------------------
//   static_Rest_call
//---------------------------------------------------------

static QScriptValue static_Rest_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Rest(): Did you forget to construct with 'new'?"));
      Rest* rest = 0;
      if (context->argumentCount() == 0)
            rest = new Rest(0);
      else if (context->argumentCount() == 1) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            rest   = new Rest(score);
            }
      if (rest)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(rest));
      return context->throwError(QString::fromLatin1("Rest(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_Rest_class
//---------------------------------------------------------

QScriptValue create_Rest_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &restInterface;

      engine->setDefaultPrototype(qMetaTypeId<Rest*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Rest*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Rest_call, function_lengths_rest[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Rest*>(), proto);
      return engine->newFunction(static_Rest_call, proto, 1);
      }

