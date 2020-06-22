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

#ifndef __TUPLET_H__
#define __TUPLET_H__

#include "duration.h"
#include "property.h"

namespace Ms {

class Text;
class Spanner;
enum class TupletNumberType  : char;
enum class TupletBracketType : char;

//------------------------------------------------------------------------
//   @@ Tuplet
//!     Example of 1/8 triplet:
//!       _baseLen     = 1/8  (tuplet is measured in eighth notes)
//!       _ratio       = 3/2  (3 eighth tuplet notes played in the space of 2 regular eighth notes)
//!
//!    Entire tuplet has a duration of _baseLen * _ratio.denominator().
//!    A single tuplet note has duration of _baseLen * _ratio.denominator() / _ratio.numerator().
//------------------------------------------------------------------------

class Tuplet final : public DurationElement {
      std::vector<DurationElement*> _elements;
      Direction _direction;
      TupletNumberType _numberType;
      TupletBracketType _bracketType;
      Spatium _bracketWidth;

      bool _hasBracket;
      Fraction _ratio;
      TDuration _baseLen;      // 1/8 for a triplet of 1/8

      bool _isUp;

      Fraction _tick;

      QPointF p1, p2;
      QPointF _p1, _p2;       // user offset
      mutable int _id;        // used during read/write

      Text* _number;
      QPointF bracketL[4];
      QPointF bracketR[3];

      Fraction addMissingElement(const Fraction& startTick, const Fraction& endTick);

   public:
      Tuplet(Score*);
      Tuplet(const Tuplet&);
      ~Tuplet();

      Tuplet* clone() const override    { return new Tuplet(*this);   }
      ElementType type() const override { return ElementType::TUPLET; }
      void setTrack(int val) override;

      void add(Element*) override;
      void remove(Element*) override;

      Text* number() const    { return _number; }
      void setNumber(Text* t) { _number = t; }
      void resetNumberProperty();

      bool isEditable() const override;
      void startEditDrag(EditData&) override;
      void editDrag(EditData&) override;

      void setSelected(bool f) override;

      Measure* measure() const override  { return toMeasure(parent()); }

      TupletNumberType numberType() const        { return _numberType;       }
      TupletBracketType bracketType() const      { return _bracketType;      }
      void setNumberType(TupletNumberType val)   { _numberType = val;        }
      void setBracketType(TupletBracketType val) { _bracketType = val;       }
      bool hasBracket() const                    { return _hasBracket;       }
      void setHasBracket(bool b)                 { _hasBracket = b;          }
      Spatium bracketWidth() const               { return _bracketWidth;     }
      void setBracketWidth(Spatium s)            { _bracketWidth = s;        }

      Fraction ratio() const                     { return _ratio;         }
      void setRatio(const Fraction& r)           { _ratio = r;            }

      const std::vector<DurationElement*>& elements() const { return _elements; }
      void clear()                                          { _elements.clear(); }
      bool contains(const DurationElement* el) const { return std::find(_elements.begin(), _elements.end(), el) != _elements.end(); }

      void layout() override;
      void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      void read(XmlReader&) override;
      void write(XmlWriter&) const override;
      bool readProperties(XmlReader&) override;

      void reset() override;

      void draw(QPainter*) const override;
      int id() const                       { return _id;          }
      void setId(int i) const              { _id = i;             }

      TDuration baseLen() const            { return _baseLen;     }
      void setBaseLen(const TDuration& d)  { _baseLen = d;        }

      void dump() const override;

      void setDirection(Direction d)          { _direction = d; }
      Direction direction() const             { return _direction; }
      bool isUp() const                       { return _isUp; }
      Fraction tick() const override  { return _tick; }
      Fraction rtick() const override;
      void setTick(const Fraction& v)         { _tick = v; }
      Fraction elementsDuration();
      void sortElements();
      bool cross() const;

      void setVisible(bool f) override;

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant& v) override;
      QVariant propertyDefault(Pid id) const override;

      Shape shape() const override;

      Element::EditBehavior normalModeEditBehavior() const override { return Element::EditBehavior::Edit; }
      int gripsCount() const override { return 2; }
      Grip initialEditModeGrip() const override { return Grip::END; }
      Grip defaultGrip() const override { return Grip::START; }
      std::vector<QPointF> gripsPositions(const EditData&) const override;

      void sanitizeTuplet();
      void addMissingElements();
      };


}     // namespace Ms
#endif

