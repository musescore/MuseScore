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

class QPainter;

namespace Ms {

class MuseScoreView;
class System;

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

   public:
      Bracket(Score*);
      virtual Bracket* clone() const override   { return new Bracket(*this); }
      virtual Element::Type type() const override { return Element::Type::BRACKET;  }

      BracketType bracketType() const    { return _bracketType; }
      void setBracketType(BracketType t) { _bracketType = t;    }

      int firstStaff() const           { return _firstStaff; }
      void setFirstStaff(int val)      { _firstStaff = val;  }

      int lastStaff() const            { return _lastStaff; }
      void setLastStaff(int val)       { _lastStaff = val;  }

      int level() const                { return _column;           }
      void setLevel(int v)             { _column = v;              }
      int span() const                 { return _span;             }
      void setSpan(int v)              { _span = v;                }
      System* system() const           { return (System*)parent(); }

      virtual void setHeight(qreal) override;
      virtual qreal width() const override;

      virtual void draw(QPainter*) const override;
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;

      virtual bool isEditable() const override { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual bool edit(MuseScoreView*, Grip, int, Qt::KeyboardModifiers, const QString&) override;
      virtual void endEdit() override;
      virtual void editDrag(const EditData&) override;
      virtual void endEditDrag() override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 1; }
      virtual QPointF gripAnchor(Grip) const override;

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };


}     // namespace Ms
#endif

