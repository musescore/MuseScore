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

#ifndef __REST_H__
#define __REST_H__

#include "chordrest.h"
#include "notedot.h"

namespace Ms {

class TDuration;
enum class SymId;

//---------------------------------------------------------
//    @@ Rest
///     This class implements a rest.
//---------------------------------------------------------

class Rest : public ChordRest {
      // values calculated by layout:
      SymId _sym;
      int dotline    { -1  };       // depends on rest symbol
      qreal _mmWidth;               // width of multi measure rest
      bool _gap      { false };     // invisible and not selectable for user
      std::vector<NoteDot*> _dots;

      virtual QRectF drag(EditData&) override;
      virtual qreal upPos()   const override;
      virtual qreal downPos() const override;
      virtual void setOffset(const QPointF& o) override;


   public:
      Rest(Score* s = 0);
      Rest(Score*, const TDuration&);
      Rest(const Rest&, bool link = false);
      ~Rest() { qDeleteAll(_dots); }

      virtual ElementType type() const override { return ElementType::REST; }
      Rest &operator=(const Rest&) = delete;

      virtual Rest* clone() const override        { return new Rest(*this, false); }
      virtual Element* linkedClone()              { return new Rest(*this, true); }
      virtual Measure* measure() const override   { return parent() ? (Measure*)(parent()->parent()) : 0; }
      virtual qreal mag() const override;
      virtual void draw(QPainter*) const override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      void setTrack(int val);

      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;
      virtual void layout() override;

      bool isGap() const               { return _gap;     }
      virtual void setGap(bool v)      { _gap = v;        }

      virtual void reset() override;

      virtual void add(Element*);
      virtual void remove(Element*);

      virtual void read(XmlReader&) override;
      virtual void write(XmlWriter& xml) const override;

      void layoutMMRest(qreal val);
      qreal mmWidth() const        { return _mmWidth; }
      SymId getSymbol(TDuration::DurationType type, int line, int lines,  int* yoffset);

      void checkDots();
      void layoutDots();
      NoteDot* dot(int n);
      int getDotline() const   { return dotline; }
      static int getDotline(TDuration::DurationType durationType);
      SymId sym() const        { return _sym;    }
      bool accent();
      void setAccent(bool flag);
      int computeLineOffset(int lines);

      virtual int upLine() const;
      virtual int downLine() const;
      virtual QPointF stemPos() const;
      virtual qreal stemPosX() const;
      virtual QPointF stemPosBeam() const;

      virtual bool setProperty(Pid propertyId, const QVariant& v) override;
      virtual QVariant getProperty(Pid propertyId) const override;
      virtual QVariant propertyDefault(Pid) const override;

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;
      virtual QString accessibleInfo() const override;
      virtual QString screenReaderInfo() const override;
      Shape shape() const override;
      };

}     // namespace Ms
#endif

