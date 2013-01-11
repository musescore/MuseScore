//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: pedal.h 5149 2011-12-29 08:38:43Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PEDAL_H__
#define __PEDAL_H__

#include "textline.h"

//---------------------------------------------------------
//   @@ PedalSegment
//---------------------------------------------------------

class PedalSegment : public TextLineSegment {
      Q_OBJECT

   protected:

   public:
      PedalSegment(Score* s) : TextLineSegment(s) {}
      virtual ElementType type() const     { return PEDAL_SEGMENT; }
      virtual PedalSegment* clone() const  { return new PedalSegment(*this); }
      virtual void layout();
      };

//---------------------------------------------------------
//   @@ Pedal
//---------------------------------------------------------

class Pedal : public TextLine {
      Q_OBJECT

   public:
      Pedal(Score* s);
      virtual Pedal* clone() const     { return new Pedal(*this); }
      virtual ElementType type() const { return PEDAL; }
      virtual void read(XmlReader&);
      LineSegment* createLineSegment();
      virtual void setYoff(qreal);
      };
#endif

