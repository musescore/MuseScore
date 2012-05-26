//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scmeasure.cpp 5149 2011-12-29 08:38:43Z wschweer $
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
#include "libmscore/measure.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/page.h"
#include "script.h"
#include "libmscore/system.h"

Q_DECLARE_METATYPE(Measure*);
Q_DECLARE_METATYPE(Score*);

static const char* const function_names_measure[] = {
      "lineBreak", "pageNumber", "boundingRect", "pos"
      };
static const int function_lengths_measure[] = {
      1, 1, 0, 0
      };
static const QScriptValue::PropertyFlags flags_measure[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration
      };

ScriptInterface measureInterface = {
      4,
      function_names_measure,
      function_lengths_measure,
      flags_measure
      };

//---------------------------------------------------------
//   prototype_Measure_call
//---------------------------------------------------------

static QScriptValue prototype_Measure_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Measure* measure = qscriptvalue_cast<Measure*>(context->thisObject());
      if (!measure) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Measure.%0(): this object is not a Measure")
               .arg(function_names_measure[_id]));
            }
      switch(_id) {
            case 0:     // "lineBreak",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), measure->lineBreak());
                  else if (context->argumentCount() == 1) {
                        Score* score = measure->score();
                        bool val = context->argument(0).toBool();
                        bool lineb = measure->lineBreak();
                        if (lineb == val)
                              return context->engine()->undefinedValue();
                        if (val) {
                	            LayoutBreak* lb = new LayoutBreak(score);
                              lb->setSubtype(LAYOUT_BREAK_LINE);
                              lb->setTrack(-1);       // this are system elements
                              lb->setParent(measure);
                              score->undoAddElement(lb);
                              }
                        else {
                              // remove line break
                              foreach(Element* e, *measure->el()) {
                                    if (e->type() == LAYOUT_BREAK && static_cast<LayoutBreak*>(e)->subtype() == LAYOUT_BREAK_LINE) {
                                          measure->score()->undoRemoveElement(e);
                                          }
                                    }
                              }
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 1:     // "pageNumber",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), measure->system()->page()->no());
                  break;
            case 2:     // "boundingRect"
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), measure->bbox());
                  break;
            case 3:     // "pos"
                  if (context->argumentCount() == 0){
                        Page* page = (Page*)measure->parent()->parent();
                        QPointF pos(measure->pagePos().x() - page->pagePos().x(),  measure->pagePos().y());
                        return qScriptValueFromValue(context->engine(), pos);
                        }
                  break;

            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Note.%0(): bad argument count or value")
         .arg(function_names_measure[_id]));
      }

//---------------------------------------------------------
//   static_Measure_call
//---------------------------------------------------------

static QScriptValue static_Measure_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Measure(): Did you forget to construct with 'new'?"));
      Measure* measure = 0;
      if (context->argumentCount() == 0)
            measure = new Measure(0);
      else if (context->argumentCount() == 1) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            measure   = new Measure(score);
            }
      if (measure)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(measure));
      return context->throwError(QString::fromLatin1("Measure(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_Measure_class
//---------------------------------------------------------

QScriptValue create_Measure_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &measureInterface;

      engine->setDefaultPrototype(qMetaTypeId<Measure*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Measure*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Measure_call, function_lengths_measure[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Measure*>(), proto);
      return engine->newFunction(static_Measure_call, proto, 1);
      }
