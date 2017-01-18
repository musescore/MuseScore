//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BRACKET_H__
#define __BRACKET_H__

#include "element.h"

namespace Ms {

class MuseScoreView;
class System;
enum class BracketType : signed char;

//---------------------------------------------------------
//   @@ Bracket
//---------------------------------------------------------

class Bracket : public Element {
      Q_OBJECT

      BracketType _bracketType;

      qreal h2;

      int _column;
      int _span;
      int _firstStaff;
      int _lastStaff;

      QPainterPath path;
      SymId _braceSymbol;
      Shape _shape;

      // horizontal scaling factor for brace symbol. Cannot be equal to magY or depend on h
      // because layout needs width of brace before knowing height of system...
      qreal _magx;

   public:
      Bracket(Score*);
      virtual Bracket* clone() const override   { return new Bracket(*this); }
      virtual ElementType type() const override { return ElementType::BRACKET;  }

      BracketType bracketType() const    { return _bracketType; }
      void setBracketType(BracketType t) { _bracketType = t;    }

      int firstStaff() const             { return _firstStaff; }
      void setFirstStaff(int val)        { _firstStaff = val;  }

      int lastStaff() const              { return _lastStaff; }
      void setLastStaff(int val)         { _lastStaff = val;  }

      int level() const                  { return _column;           }
      void setLevel(int v)               { _column = v;              }
      int span() const                   { return _span;             }
      void setSpan(int v);
      System* system() const             { return (System*)parent(); }

      virtual void setHeight(qreal) override;
      virtual qreal width() const override;

      virtual Shape shape() const override { return _shape; }

      virtual void draw(QPainter*) const override;
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;

      virtual bool isEditable() const override { return true; }
      virtual bool edit(MuseScoreView*, Grip, int, Qt::KeyboardModifiers, const QString&) override;
      virtual void endEdit() override;
      virtual void editDrag(const EditData&) override;
      virtual void endEditDrag(const EditData&) override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 1; }

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms
#endif

