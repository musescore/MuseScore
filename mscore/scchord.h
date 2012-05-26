//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scchord.h 1936 2009-07-17 16:12:47Z wschweer $
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

#ifndef __SCCHORD_H__
#define __SCCHORD_H__

#include "scchordrest.h"

class Chord;
class Note;
class Score;
typedef Note* NotePtr;
class Note;

//---------------------------------------------------------
//   ScChord
//---------------------------------------------------------

class ScChord : public QObject, public QScriptClass
      {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const ChordPtr& ba);
      static void fromScriptValue(const QScriptValue &obj, ChordPtr& ba);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScChord(QScriptEngine* se);
      ~ScChord() {}

      QScriptValue constructor()     { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const ChordPtr&);
      QString name() const           { return QLatin1String("Chord"); }
      QScriptValue prototype() const { return proto; }
      };

//---------------------------------------------------------
//   ScChordPrototype
//---------------------------------------------------------

class ScChordPrototype : public ScChordRestPrototype
      {
      Chord* thisChord() const;

   public:
      ScChordPrototype(QObject *parent = 0) : ScChordRestPrototype(parent) {}
      ~ScChordPrototype() {}
      };

Q_DECLARE_METATYPE(ScChord*)

#endif
