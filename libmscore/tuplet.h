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

class QPainter;

namespace Ms {

class Text;
class Spanner;

//------------------------------------------------------------------------
//   @@ Tuplet
//!     Example of 1/8 triplet:
//!       _baseLen     = 1/8
//!       _actualNotes = 3
//!       _normalNotes = 2     (3 notes played in the time of 2/8)
//!
//!    The tuplet has a len of _baseLen * _normalNotes.
//!    A tuplet note has len of _baseLen * _normalNotes / _actualNotes.
//------------------------------------------------------------------------

class Tuplet : public DurationElement {
      Q_OBJECT

      int _tick;

   public:
      enum class NumberType : char { SHOW_NUMBER, SHOW_RELATION, NO_TEXT };
      enum class BracketType : char { AUTO_BRACKET, SHOW_BRACKET, SHOW_NO_BRACKET };

   private:
      std::vector<DurationElement*> _elements;

      Direction _direction;
      NumberType _numberType;
      BracketType _bracketType;
      PropertyStyle directionStyle  { PropertyStyle::STYLED };
      PropertyStyle numberStyle     { PropertyStyle::STYLED };
      PropertyStyle bracketStyle    { PropertyStyle::STYLED };

      bool _hasBracket;

      Fraction _ratio;
      TDuration _baseLen;      // 1/8 for a triplet of 1/8

      bool _isUp;

      QPointF p1, p2;
      QPointF _p1, _p2;       // user offset
      mutable int _id;        // used during read/write

      Text* _number;
      QPointF bracketL[4];
      QPointF bracketR[3];

   public:
      Tuplet(Score*);
      Tuplet(const Tuplet&);
      ~Tuplet();
      virtual Tuplet* clone() const override      { return new Tuplet(*this); }
      virtual Element::Type type() const override { return Element::Type::TUPLET; }
      virtual void setTrack(int val) override;

      virtual void add(Element*) override;
      virtual void remove(Element*) override;

      virtual bool isEditable() const override;
      virtual void editDrag(const EditData&) override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 2; }

      virtual void setSelected(bool f) override;

      virtual Measure* measure() const override { return (Measure*)parent(); }

      NumberType numberType() const        { return _numberType;       }
      BracketType bracketType() const      { return _bracketType;      }
      void setNumberType(NumberType val)   { _numberType = val;        }
      void setBracketType(BracketType val) { _bracketType = val;       }
      bool hasBracket() const              { return _hasBracket;       }

      Fraction ratio() const               { return _ratio;         }
      void setRatio(const Fraction& r)     { _ratio = r;            }

      const std::vector<DurationElement*>& elements() const { return _elements; }
      void clear()                         { _elements.clear(); }

      virtual void layout() override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      Text* number() const { return _number; }

      virtual void read(XmlReader&) override;
      virtual void write(Xml&) const override;

      virtual void reset() override;

      virtual void draw(QPainter*) const override;
      int id() const                       { return _id;          }
      void setId(int i) const              { _id = i;             }

      TDuration baseLen() const            { return _baseLen;     }
      void setBaseLen(const TDuration& d)  { _baseLen = d;        }

      virtual void dump() const override;

      void setDirection(Direction d)       { _direction = d; }
      Direction direction() const          { return _direction; }
      bool isUp() const                    { return _isUp; }
      virtual int tick() const override    { return _tick; }
      void setTick(int val)                { _tick = val; }
      Fraction elementsDuration();
      void sortElements();

      virtual void setVisible(bool f) override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant& v) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;
      virtual StyleIdx getPropertyStyle(P_ID) const override;
      };


}     // namespace Ms
#endif

