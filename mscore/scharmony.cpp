//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scnote.cpp 1840 2009-05-20 11:57:51Z wschweer $
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

#include "musescore.h"
#include "libmscore/harmony.h"
#include "libmscore/utils.h"
#include "libmscore/undo.h"
#include "script.h"

Q_DECLARE_METATYPE(Harmony*);
Q_DECLARE_METATYPE(Score*);

static const char* const function_names_harmony[] = {
      "id", "root", "base"
      };
static const int function_lengths_harmony[] = {
      1, 1, 1
      };
static const QScriptValue::PropertyFlags flags_harmony[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      };

ScriptInterface harmonyInterface = {
      3,
      function_names_harmony,
      function_lengths_harmony,
      flags_harmony
      };

//---------------------------------------------------------
//   prototype_Harmony_call
//---------------------------------------------------------

static QScriptValue prototype_Harmony_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Harmony* harmony = qscriptvalue_cast<Harmony*>(context->thisObject());
      if (!harmony) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Harmony.%0(): this object is not a Harmony")
               .arg(function_names_harmony[_id]));
            }
      switch(_id) {
            case 0:     // "id",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), harmony->id());
                  else if (context->argumentCount() == 1) {
                        int val = context->argument(0).toInt32();
                        harmony->setId(val);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 1:     // "root",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), harmony->rootTpc());
                  else if (context->argumentCount() == 1) {
                        int val = context->argument(0).toInt32();
                        harmony->setRootTpc(val);
                        return context->engine()->undefinedValue();
                        }
            case 2:     // "base"
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), harmony->baseTpc());
                  else if (context->argumentCount() == 1) {
                        int val = context->argument(0).toInt32();
                        harmony->setBaseTpc(val);
                        return context->engine()->undefinedValue();
                        }
                  break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Note.%0(): bad argument count or value")
         .arg(function_names_harmony[_id]));
      }

//---------------------------------------------------------
//   static_Harmony_call
//---------------------------------------------------------

static QScriptValue static_Harmony_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Harmony(): Did you forget to construct with 'new'?"));
      Harmony* harmony = 0;
      if (context->argumentCount() == 0)
            harmony = new Harmony(0);
      else if (context->argumentCount() == 1) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            harmony   = new Harmony(score);
            }
      if (harmony)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(harmony));
      return context->throwError(QString::fromLatin1("Harmony(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_Harmony_class
//---------------------------------------------------------

QScriptValue create_Harmony_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &harmonyInterface;

      engine->setDefaultPrototype(qMetaTypeId<Harmony*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Harmony*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Harmony_call, function_lengths_harmony[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Harmony*>(), proto);
      return engine->newFunction(static_Harmony_call, proto, 1);
      }
