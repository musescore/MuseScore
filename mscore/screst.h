//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scchord.h 1840 2009-05-20 11:57:51Z wschweer $
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

#ifndef __SCREST_H__
#define __SCREST_H__

#include "scchordrest.h"

class Rest;
class Score;
typedef Rest* RestPtr;

//---------------------------------------------------------
//   ScRest
//---------------------------------------------------------

class ScRest : public QObject, public QScriptClass
      {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const RestPtr& ba);
      static void fromScriptValue(const QScriptValue &obj, RestPtr& ba);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScRest(QScriptEngine* se);
      ~ScRest() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const RestPtr&);
      QString name() const           { return QLatin1String("Rest"); }
      QScriptValue prototype() const { return proto; }
      };

//---------------------------------------------------------
//   ScRestPrototype
//---------------------------------------------------------

class ScRestPrototype : public ScChordRestPrototype
      {
      Q_OBJECT
      Rest* thisRest() const;

   public slots:

   public:
      ScRestPrototype(QObject *parent = 0) : ScChordRestPrototype(parent) {}
      ~ScRestPrototype() {}
      };

Q_DECLARE_METATYPE(ScRest*)
Q_DECLARE_METATYPE(RestPtr)
Q_DECLARE_METATYPE(RestPtr*)

#endif
