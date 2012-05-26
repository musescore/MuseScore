//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __SCHARMONY_H__
#define __SCHARMONY_H__

class Harmony;
class Score;
typedef Harmony* HarmonyPtr;

//---------------------------------------------------------
//   ScHarmony
//---------------------------------------------------------

class ScHarmony : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const HarmonyPtr& ba);
      static void fromScriptValue(const QScriptValue &obj, HarmonyPtr& ba);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScHarmony(QScriptEngine* se);
      ~ScHarmony() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const HarmonyPtr&);
      QueryFlags queryProperty(const QScriptValue& object,
         const QScriptString& name, QueryFlags flags, uint* id);
      QScriptValue property(const QScriptValue& obhect,
         const QScriptString& name, uint id);
      virtual void setProperty(QScriptValue& object, const QScriptString& name,
         uint id, const QScriptValue& value);
      QScriptValue::PropertyFlags propertyFlags(
         const QScriptValue& object, const QScriptString& name, uint id);
      QScriptClassPropertyIterator* newIterator(const QScriptValue& object);
      QString name() const           { return QLatin1String("Harmony"); }
      QScriptValue prototype() const { return proto; }
      };

//---------------------------------------------------------
//   ScHarmonyPrototype
//---------------------------------------------------------

class ScHarmonyPrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      Q_PROPERTY(int id READ getId WRITE setId SCRIPTABLE true)
      Q_PROPERTY(int root READ getRoot WRITE setRoot SCRIPTABLE true)
      Q_PROPERTY(int base READ getBase WRITE setBase SCRIPTABLE true)

      Harmony* thisHarmony() const;

   public:
      ScHarmonyPrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScHarmonyPrototype() {}
      int getId() const;
      void setId(int);
      int getRoot() const;
      void setRoot(int);
      int getBase() const;
      void setBase(int);

   public slots:
      };

Q_DECLARE_METATYPE(HarmonyPtr)
Q_DECLARE_METATYPE(HarmonyPtr*)
Q_DECLARE_METATYPE(ScHarmony*)

#endif


