//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: breath.h 5504 2012-03-29 11:01:37Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BREATH_H__
#define __BREATH_H__

#include "element.h"

class QPainter;

//---------------------------------------------------------
//   @@ Breath
///    brathT() is index in symList
//---------------------------------------------------------

class Breath : public Element {
      Q_OBJECT

      int _breathType;
      static const int breathSymbols = 4;
      static int symList[breathSymbols];

   public:
      Breath(Score* s);
      virtual Breath* clone() const { return new Breath(*this); }

      virtual ElementType type() const { return BREATH; }
      int breathType() const    { return _breathType; }
      void setBreathType(int v) { _breathType = v; }

      Segment* segment() const         { return (Segment*)parent(); }
      virtual Space space() const;

      virtual void draw(QPainter*) const;
      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      virtual QPointF pagePos() const;      ///< position in page coordinates
      };

#endif

