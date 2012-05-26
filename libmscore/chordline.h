//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: chordline.h 5491 2012-03-22 20:19:22Z lvinken $
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CHORDLINE_H__
#define __CHORDLINE_H__

#include "element.h"

class Chord;
class QPainter;

// subtypes:
enum ChordLineType {
      CHORDLINE_NOTYPE, CHORDLINE_FALL, CHORDLINE_DOIT
      };

//---------------------------------------------------------
//   ChordLine
//    bezier line attached to top note of a chord
//    implements fall, doit, plop, bend
//---------------------------------------------------------

class ChordLine : public Element {
      ChordLineType _subtype;
      QPainterPath path;
      bool modified;

   public:
      ChordLine(Score*);
      ChordLine(const ChordLine&);

      virtual ChordLine* clone() const { return new ChordLine(*this); }
      virtual ElementType type() const { return CHORDLINE; }
      virtual void setSubtype(ChordLineType);
      ChordLineType subtype() const    { return _subtype; }
      Chord* chord() const             { return (Chord*)(parent()); }

      virtual void read(const QDomElement&);
      virtual void write(Xml& xml) const;
      virtual void layout();
      virtual void draw(QPainter*) const;

      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      };

#endif

