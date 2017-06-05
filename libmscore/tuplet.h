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
      QList<DurationElement*> _elements;
      NumberType _numberType;
      BracketType _bracketType;
      bool _hasBracket;

      Fraction _ratio;
      TDuration _baseLen;      // 1/8 for a triplet of 1/8

      MScore::Direction _direction;
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
      virtual Tuplet* clone() const      { return new Tuplet(*this); }
      virtual Element::Type type() const { return Element::Type::TUPLET; }
      virtual void setTrack(int val);

      virtual void add(Element*);
      virtual void remove(Element*);

      virtual bool isEditable() const;
      virtual void editDrag(const EditData&);
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 2; }

      virtual void setSelected(bool f);

      virtual Measure* measure() const { return (Measure*)parent(); }

      NumberType numberType() const        { return _numberType;       }
      BracketType bracketType() const      { return _bracketType;      }
      void setNumberType(NumberType val)   { _numberType = val;        }
      void setBracketType(BracketType val) { _bracketType = val;       }
      bool hasBracket() const              { return _hasBracket;       }

      Fraction ratio() const           { return _ratio;         }
      void setRatio(const Fraction& r) { _ratio = r;            }

      const QList<DurationElement*>& elements() const { return _elements; }
      void clear()                                    { _elements.clear(); }

      virtual void layout();
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      Text* number() const { return _number; }

      void read(XmlReader&);
      void write(Xml&) const;

      virtual void reset();

      virtual void draw(QPainter*) const;
      int id() const                       { return _id;          }
      void setId(int i) const              { _id = i;             }

      TDuration baseLen() const            { return _baseLen;     }
      void setBaseLen(const TDuration& d)  { _baseLen = d;        }

      virtual void dump() const;

      void setDirection(MScore::Direction d)       { _direction = d; }
      MScore::Direction direction() const          { return _direction; }
      bool isUp() const                    { return _isUp; }
      virtual int tick() const             { return _tick; }
      void setTick(int val)                { _tick = val; }
      void sortElements();
      Fraction elementsDuration();

      virtual void setVisible(bool f);
      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant& v);
      QVariant propertyDefault(P_ID id) const;
      void sanitizeTuplet();
      };


}     // namespace Ms
#endif

