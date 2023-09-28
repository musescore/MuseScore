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
#include "sym.h"

namespace Ms {

class TDuration;

//---------------------------------------------------------
//    @@ Rest
///     This class implements a rest.
//---------------------------------------------------------

class Rest : public ChordRest {
      // values calculated by layout:
      SymId _sym;
      int dotline    { -1  };       // depends on rest symbol
      qreal _mmWidth;               // width of multimeasure rest
      qreal _mmRestNumberPos;       // vertical position of number of multimeasure rest
      bool _gap      { false };     // invisible and not selectable for user
      std::vector<NoteDot*> _dots;

      QRectF drag(EditData&) override;
      qreal upPos() const override;
      qreal downPos() const override;
      void setOffset(const QPointF& o) override;
      Sid getPropertyStyle(Pid pid) const override;


   public:
      Rest(Score* s = 0);
      Rest(Score*, const TDuration&);
      Rest(const Rest&, bool link = false);
      ~Rest() { qDeleteAll(_dots); }

      virtual ElementType type() const override { return ElementType::REST; }
      Rest &operator=(const Rest&) = delete;

      Rest* clone() const override        { return new Rest(*this, false); }
      Element* linkedClone() override     { return new Rest(*this, true); }
      Measure* measure() const override   { return parent() ? toMeasure(parent()->parent()) : 0; }
      qreal mag() const override;
      void draw(QPainter*) const override;
      void scanElements(void* data, void (*func)(void*, Element*), bool all = true) override;
      void setTrack(int val);

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;
      void layout() override;

      bool isGap() const               { return _gap;     }
      virtual void setGap(bool v)      { _gap = v;        }

      virtual void add(Element*);
      virtual void remove(Element*);

      void read(XmlReader&) override;
      void write(XmlWriter& xml) const override;

      void layoutMMRest(qreal val);
      QRectF mmRestNumberRect() const;
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
      virtual qreal rightEdge() const override;
      qreal centerX() const;

      void localSpatiumChanged(qreal oldValue, qreal newValue) override;
      QVariant propertyDefault(Pid) const override;
      void resetProperty(Pid id);
      bool setProperty(Pid propertyId, const QVariant& v) override;
      QVariant getProperty(Pid propertyId) const override;
      void undoChangeDotsVisible(bool v);

      Element* nextElement() override;
      Element* prevElement() override;
      QString accessibleInfo() const override;
      QString screenReaderInfo() const override;
      Shape shape() const override;
      void editDrag(EditData& editData) override;
      };

}     // namespace Ms
#endif

