//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: sctext.h 1840 2009-05-20 11:57:51Z wschweer $
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

#ifndef __SCTEXT_H__
#define __SCTEXT_H__

class Score;
class Text;
typedef Text* TextPtr;

//---------------------------------------------------------
//   ScText
//---------------------------------------------------------

class ScText : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const TextPtr& ba);
      static void fromScriptValue(const QScriptValue &obj, TextPtr& ba);

      QScriptString textText, textSize, textDefaultFont, textColor;
      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScText(QScriptEngine* se);
      ~ScText() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const TextPtr&);
      QueryFlags queryProperty(const QScriptValue& object,
         const QScriptString& name, QueryFlags flags, uint* id);
      QScriptValue property(const QScriptValue& obhect,
         const QScriptString& name, uint id);
      virtual void setProperty(QScriptValue& object, const QScriptString& name,
         uint id, const QScriptValue& value);
      QScriptValue::PropertyFlags propertyFlags(
         const QScriptValue& object, const QScriptString& name, uint id);
      QScriptClassPropertyIterator* newIterator(const QScriptValue& object);
      QString name() const           { return QLatin1String("Text"); }
      QScriptValue prototype() const { return proto; }
      };

//---------------------------------------------------------
//   ScTextPrototype
//---------------------------------------------------------

class ScTextPrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      Text* thisText() const;

   public:
      ScTextPrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScTextPrototype() {}

   public slots:
      };

Q_DECLARE_METATYPE(TextPtr)
Q_DECLARE_METATYPE(TextPtr*)
Q_DECLARE_METATYPE(ScText*)

#endif


