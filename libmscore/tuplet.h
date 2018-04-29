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
//!       _baseLen     = 1/8
//!       _actualNotes = 3
//!       _normalNotes = 2     (3 notes played in the time of 2/8)
//!
//!    The tuplet has a  len of _baseLen * _normalNotes.
//!    A tuplet note has len of _baseLen * _normalNotes / _actualNotes.
//------------------------------------------------------------------------

class Tuplet final : public DurationElement {
      // the tick position of a tuplet is the tick position of its
      // first element:
      int _tick;
      std::vector<DurationElement*> _elements;
      Direction _direction;
      TupletNumberType _numberType;
      TupletBracketType _bracketType;
      Spatium _bracketWidth;

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
      virtual Tuplet* clone() const override    { return new Tuplet(*this);   }
      virtual ElementType type() const override { return ElementType::TUPLET; }
      virtual void setTrack(int val) override;

      virtual void add(Element*) override;
      virtual void remove(Element*) override;

      Text* number() const    { return _number; }
      void setNumber(Text* t) { _number = t; }
      void resetNumberProperty();

      virtual bool isEditable() const override;
      virtual void startEdit(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;

      virtual void setSelected(bool f) override;

      virtual Measure* measure() const override { return (Measure*)parent(); }

      TupletNumberType numberType() const  { return _numberType;       }
      TupletBracketType bracketType() const { return _bracketType;      }
      void setNumberType(TupletNumberType val)   { _numberType = val;        }
      void setBracketType(TupletBracketType val) { _bracketType = val;       }
      bool hasBracket() const              { return _hasBracket;       }
      void setHasBracket(bool b)           { _hasBracket = b;          }
      Spatium bracketWidth() const         { return _bracketWidth;     }
      void setBracketWidth(Spatium s)      { _bracketWidth = s;        }
      QPointF* getLeftBracket()            { return bracketL;}
      QPointF* getRightBracket()           { return bracketR;}
      Text* getText()                      { return _number;}

      Fraction ratio() const               { return _ratio;         }
      void setRatio(const Fraction& r)     { _ratio = r;            }

      const std::vector<DurationElement*>& elements() const { return _elements; }
      void clear()                         { _elements.clear(); }

      virtual void layout() override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      virtual void read(XmlReader&) override;
      virtual void write(XmlWriter&) const override;
      virtual bool readProperties(XmlReader&) override;

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

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant& v) override;
      virtual QVariant propertyDefault(Pid id) const override;

      virtual Shape shape() const override;

      void sanitizeTuplet();
      void addMissingElements();

   private:
      Fraction addMissingElement(int startTick, int endTick);
      };


}     // namespace Ms
#endif

