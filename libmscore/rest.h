//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: rest.h 5500 2012-03-28 16:28:26Z wschweer $
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

class TDuration;

//---------------------------------------------------------
//    @@ Rest
///   This class implements a rest.
//---------------------------------------------------------

class Rest : public ChordRest {
      Q_OBJECT

      // values calculated by layout:
      int _sym;
      int dotline;            // depends on rest symbol
      qreal _mmWidth;         // width of multi measure rest

      virtual QRectF drag(const QPointF& s);
      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void setUserOffset(qreal x, qreal y);

   public:
      Rest(Score* s = 0);
      Rest(Score*, const TDuration&);
      virtual Rest* clone() const      { return new Rest(*this); }
      virtual ElementType type() const { return REST; }

      virtual void draw(QPainter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement& d) { read(d, 0, 0); }
      virtual void read(const QDomElement&, QList<Tuplet*>*, QList<Spanner*>*);
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void layout();

      void setMMWidth(qreal val);
      qreal mmWidth() const        { return _mmWidth; }
      int getSymbol(TDuration::DurationType type, int line, int lines,  int* yoffset);

      int getDotline() const { return dotline; }
      int sym() const        { return _sym;    }
      };

#endif

