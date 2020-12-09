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
      qreal ay1;
      qreal h2;

      int _firstStaff;
      int _lastStaff;

      QPainterPath path;
      SymId _braceSymbol;
      Shape _shape;

      // horizontal scaling factor for brace symbol. Cannot be equal to magY or depend on h
      // because layout needs width of brace before knowing height of system...
      qreal _magx;
      Measure* _measure = nullptr;

   public:
      Bracket(Score*);
      ~Bracket();

      Bracket* clone() const override   { return new Bracket(*this); }
      ElementType type() const override { return ElementType::BRACKET;  }

      void setBracketItem(BracketItem* i)       { _bi = i; }
      BracketItem* bracketItem() const          { return _bi;          }

      BracketType bracketType() const           { return _bi->bracketType(); }
      static const char* bracketTypeName(BracketType type);

      int firstStaff() const                    { return _firstStaff; }
      int lastStaff() const                     { return _lastStaff; }
      void setStaffSpan(int a, int b);

      SymId braceSymbol() const                 { return _braceSymbol; }
      void setBraceSymbol(const SymId& sym)     { _braceSymbol = sym; }
      int column() const                        { return _bi->column();  }
      int span() const                          { return _bi->bracketSpan();    }
      qreal magx() const                        { return _magx;                 }

      System* system() const                    { return (System*)parent(); }

      Measure* measure() const                  { return _measure; }
      void setMeasure(Measure* measure)         { _measure = measure; }

      Fraction playTick() const override;

      void setHeight(qreal) override;
      qreal width() const override;

      Shape shape() const override { return _shape; }

      void draw(QPainter*) const override;
      void layout() override;

      void write(XmlWriter& xml) const override;
      void read(XmlReader&) override;

      bool isEditable() const override { return true; }
      void startEdit(EditData&) override;
      bool edit(EditData&) override;
      void endEdit(EditData&) override;
      void editDrag(EditData&) override;
      void endEditDrag(EditData&) override;

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

      void undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps) override;
      using ScoreElement::undoChangeProperty;

      int gripsCount() const override { return 1; }
      Grip initialEditModeGrip() const override { return Grip::START; }
      Grip defaultGrip() const override { return Grip::START; }
      std::vector<QPointF> gripsPositions(const EditData&) const override;

      void setSelected(bool f) override;
      };


}     // namespace Ms
#endif

