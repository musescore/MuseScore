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
#include "bracketItem.h"

namespace Ms {

class MuseScoreView;
class System;
enum class BracketType : signed char;

//---------------------------------------------------------
//   @@ Bracket
//---------------------------------------------------------

class Bracket final : public Element {
      BracketItem* _bi;
      qreal h2;

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
      virtual ~Bracket();
      virtual Bracket* clone() const override   { return new Bracket(*this); }
      virtual ElementType type() const override { return ElementType::BRACKET;  }

      void setBracketItem(BracketItem* i)       { _bi = i; }
      BracketItem* bracketItem() const          { return _bi;          }

      BracketType bracketType() const           { return _bi->bracketType(); }

      int firstStaff() const                    { return _firstStaff; }
      void setFirstStaff(int val)               { _firstStaff = val;  }

      int lastStaff() const                     { return _lastStaff; }
      void setLastStaff(int val)                { _lastStaff = val;  }

      int column() const                        { return _bi->column();  }
      int span() const                          { return _bi->bracketSpan();    }

      System* system() const                    { return (System*)parent(); }

      virtual void setHeight(qreal) override;
      virtual qreal width() const override;

      virtual Shape shape() const override { return _shape; }

      virtual void draw(QPainter*) const override;
      virtual void layout() override;

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;

      virtual bool isEditable() const override { return true; }
      virtual void startEdit(EditData&) override;
      virtual bool edit(EditData&) override;
      virtual void endEdit(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual void endEditDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;

      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      virtual void setSelected(bool f) override;
      };


}     // namespace Ms
#endif

