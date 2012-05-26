//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: sctext.cpp 4388 2011-06-18 13:17:58Z wschweer $
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

#include "script.h"
#include "libmscore/text.h"

Q_DECLARE_METATYPE(Text*);
Q_DECLARE_METATYPE(Score*);

static const char* const function_names_text[] = {
      "text", "defaultFont", "color", "xOffset", "yOffset"
      };
static const int function_lengths_text[] = {
      1, 1, 1, 1, 1
      };
static const QScriptValue::PropertyFlags flags_text[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
	   QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
	   QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      };

ScriptInterface textInterface = {
      5,
      function_names_text,
      function_lengths_text,
      flags_text
      };

//---------------------------------------------------------
//   prototype_Text_call
//---------------------------------------------------------

static QScriptValue prototype_Text_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Text* text = qscriptvalue_cast<Text*>(context->thisObject());
      if (!text) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Text.%0(): this object is not a Text")
               .arg(function_names_text[_id]));
            }
      switch(_id) {
            case 0:     // "text",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), text->getText());
                  else if (context->argumentCount() == 1) {
                        QString t = qscriptvalue_cast<QString>(context->argument(0));
                        text->setText(t);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 1:     // "defaultFont",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), text->font());
                  else if (context->argumentCount() == 1) {
                        QFont f = qscriptvalue_cast<QFont>(context->argument(0));
                        text->setFont(f);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 2:     // "color"
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), text->color());
                  else if (context->argumentCount() == 1) {
                        QColor c = qscriptvalue_cast<QColor>(context->argument(0));
                        text->setColor(c);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 3:     // "xOffset"
				          if (context->argumentCount() == 0)
						            return qScriptValueFromValue(context->engine(), text->xoff());
				          else if (context->argumentCount() == 1) {
            						int v = context->argument(0).toInt32();
            						text->setXoff(v);
            						return context->engine()->undefinedValue();
						            }
				          break;
			     case 4:     // "yOffset"
                  if (context->argumentCount() == 0)
						            return qScriptValueFromValue(context->engine(), text->yoff());
				          else if (context->argumentCount() == 1) {
            						int v = context->argument(0).toInt32();
            						text->setYoff(v);
            						return context->engine()->undefinedValue();
						            }
				          break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Note.%0(): bad argument count or value")
         .arg(function_names_text[_id]));
      }

//---------------------------------------------------------
//   static_Text_call
//---------------------------------------------------------

static QScriptValue static_Text_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Text(): Did you forget to construct with 'new'?"));
      Text* text = 0;
      if (context->argumentCount() == 0)
            text = new Text(0);
      else if (context->argumentCount() == 1) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            text   = new Text(score);
            }
      if (text)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(text));
      return context->throwError(QString::fromLatin1("Text(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_Text_class
//---------------------------------------------------------

QScriptValue create_Text_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &textInterface;

      engine->setDefaultPrototype(qMetaTypeId<Text*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Text*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Text_call, function_lengths_text[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Text*>(), proto);
      return engine->newFunction(static_Text_call, proto, 1);
      }


