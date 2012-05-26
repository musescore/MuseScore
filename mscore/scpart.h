//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scpart.h 2301 2009-11-04 11:08:30Z wschweer $
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

#ifndef __SCPART_H__
#define __SCPART_H__

class Part;
class Score;
typedef Part* PartPtr;

//---------------------------------------------------------
//   ScPart
//---------------------------------------------------------

class ScPart : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const PartPtr& ba);
      static void fromScriptValue(const QScriptValue &obj, PartPtr& ba);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScPart(QScriptEngine* se);
      ~ScPart() {}

      QScriptValue prototype() const { return proto; }
      QScriptValue constructor()     { return ctor; }
      QString name() const           { return QLatin1String("Part"); }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const PartPtr&);
      };

//---------------------------------------------------------
//   ScPartPrototype
//---------------------------------------------------------

class ScPartPrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      Q_PROPERTY(QString longName READ getLongName)
      Q_PROPERTY(QString shortName READ getShortName)
      Q_PROPERTY(int midiProgram READ getMidiProgram)
      Q_PROPERTY(int midiChannel READ getMidiChannel)

      Part* thisPart() const;

   public slots:
      QString getLongName() const;
      QString getShortName() const;
      int getMidiProgram() const;
      int getMidiChannel() const;

   public:
      ScPartPrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScPartPrototype() {}
      };

Q_DECLARE_METATYPE(PartPtr)
Q_DECLARE_METATYPE(PartPtr*)
Q_DECLARE_METATYPE(ScPart*)

#endif


