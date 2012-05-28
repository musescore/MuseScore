//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: tuplet.h 5500 2012-03-28 16:28:26Z wschweer $
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

class Text;
class Spanner;
class QPainter;

//------------------------------------------------------------------------
//   Tuplet
//     Example of 1/8 triplet:
//       _baseLen     = 1/8
//       _actualNotes = 3
//       _normalNotes = 2     (3 notes played in the time of 2/8)
//
//    the tuplet has a len of _baseLen * _normalNotes
//    a tuplet note has len of _baseLen * _normalNotes / _actualNotes
//------------------------------------------------------------------------

class Tuplet : public DurationElement {
      Q_OBJECT

      int _tick;

   public:
      enum { SHOW_NUMBER, SHOW_RELATION, NO_TEXT };
      enum { AUTO_BRACKET, SHOW_BRACKET, SHOW_NO_BRACKET };

   private:
      QList<DurationElement*> _elements;
      int _numberType;
      int _bracketType;
      bool _hasBracket;

      Fraction _ratio;
      TDuration _baseLen;      // 1/8 for a triplet of 1/8

      Direction _direction;
      bool _isUp;

      QPointF p1, p2;
      QPointF _p1, _p2;       // user offset
      mutable int _id;        // used during read/write

      Text* _number;
      QPointF bracketL[4];
      QPointF bracketR[3];

      void* pDirection()   { return &_direction;            }
      void* pNumberType()  { return &_numberType;           }
      void* pBracketType() { return &_bracketType;          }
      void* pNormalNotes() { return &_ratio.rdenominator(); }
      void* pActualNotes() { return &_ratio.rnumerator();   }
      void* pP1()          { return &_p1;                   }
      void* pP2()          { return &_p2;                   }

   public:
      Tuplet(Score*);
      Tuplet(const Tuplet&);
      ~Tuplet();
      virtual Tuplet* clone() const    { return new Tuplet(*this); }
      virtual ElementType type() const { return TUPLET; }
      virtual void setTrack(int val);

      virtual void add(Element*);
      virtual void remove(Element*);

      virtual bool isEditable() const;
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;

      virtual void setSelected(bool f);

      virtual Measure* measure() const { return (Measure*)parent(); }

      int numberType() const        { return _numberType;       }
      int bracketType() const       { return _bracketType;      }
      void setNumberType(int val)   { _numberType = val;        }
      void setBracketType(int val)  { _bracketType = val;       }
      bool hasBracket() const       { return _hasBracket;       }

      Fraction ratio() const           { return _ratio;         }
      void setRatio(const Fraction& r) { _ratio = r;            }

      const QList<DurationElement*>& elements() const { return _elements; }
      void clear()                  { _elements.clear(); }

      virtual void layout();
      Text* number() const { return _number; }

      void read(const QDomElement&, QList<Tuplet*>*, const QList<Spanner*>*);
      void write(Xml&) const;

      virtual void toDefault();

      virtual void draw(QPainter*) const;
      int id() const                       { return _id;          }
      void setId(int i) const              { _id = i;             }

      TDuration baseLen() const             { return _baseLen;     }
      void setBaseLen(const TDuration& d)   { _baseLen = d;        }

      virtual void dump() const;

      void setDirection(Direction d)       { _direction = d; }
      Direction direction() const          { return _direction; }
      bool isUp() const                    { return _isUp; }
      virtual int tick() const             { return _tick; }
      void setTick(int val)                { _tick = val; }
      void sortElements();

      virtual void setVisible(bool f);

      PROPERTY_DECLARATIONS(Tuplet)
      };

#endif

