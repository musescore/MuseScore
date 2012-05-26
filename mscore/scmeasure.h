//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scmeasure.h 2391 2009-11-26 10:08:35Z lasconic $
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

#ifndef __SCMEASURE_H__
#define __SCMEASURE_H__

class Measure;
class Score;
typedef Measure* MeasurePtr;

//---------------------------------------------------------
//   ScMeasure
//---------------------------------------------------------

class ScMeasure : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const MeasurePtr& ba);
      static void fromScriptValue(const QScriptValue &obj, MeasurePtr& ba);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScMeasure(QScriptEngine* se);
      ~ScMeasure() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const MeasurePtr&);
      QueryFlags queryProperty(const QScriptValue& object,
         const QScriptString& name, QueryFlags flags, uint* id);
      QScriptValue property(const QScriptValue& obhect,
         const QScriptString& name, uint id);
      virtual void setProperty(QScriptValue& object, const QScriptString& name,
         uint id, const QScriptValue& value);
      QScriptValue::PropertyFlags propertyFlags(
         const QScriptValue& object, const QScriptString& name, uint id);
      QScriptClassPropertyIterator* newIterator(const QScriptValue& object);
      QString name() const           { return QLatin1String("Measure"); }
      QScriptValue prototype() const { return proto; }
      };

//---------------------------------------------------------
//   ScMeasurePrototype
//---------------------------------------------------------

class ScMeasurePrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      Q_PROPERTY(bool lineBreak READ getLineBreak WRITE setLineBreak SCRIPTABLE true)
      Q_PROPERTY(int pageNumber READ getPageNumber SCRIPTABLE true)
      Q_PROPERTY(double x READ getX SCRIPTABLE true)
      Q_PROPERTY(double y READ getY SCRIPTABLE true)
      Q_PROPERTY(double width READ getWidth SCRIPTABLE true)
      Q_PROPERTY(double height READ getHeight SCRIPTABLE true)
      
      Measure* thisMeasure() const;

   public:
      ScMeasurePrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScMeasurePrototype() {}
      bool getLineBreak() const;
      void setLineBreak(bool v);
      int getPageNumber() const;
      double getX() const;
      double getY() const;
      double getWidth() const;
      double getHeight() const;

   public slots:
      };

Q_DECLARE_METATYPE(MeasurePtr)
Q_DECLARE_METATYPE(MeasurePtr*)
Q_DECLARE_METATYPE(ScMeasure*)

#endif


