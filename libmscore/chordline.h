//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
      CHORDLINE_NOTYPE, CHORDLINE_FALL, CHORDLINE_DOIT,
      CHORDLINE_PLOP, CHORDLINE_SCOOP
      };

//---------------------------------------------------------
//   @@ ChordLine
///    bezier line attached to top note of a chord
///    implements fall, doit, plop, bend
//---------------------------------------------------------

class ChordLine : public Element {
      Q_OBJECT

      ChordLineType _chordLineType;
      QPainterPath path;
      bool modified;

   public:
      ChordLine(Score*);
      ChordLine(const ChordLine&);

      virtual ChordLine* clone() const { return new ChordLine(*this); }
      virtual ElementType type() const { return CHORDLINE; }
      virtual void setChordLineType(ChordLineType);
      ChordLineType chordLineType() const { return _chordLineType; }
      Chord* chord() const                { return (Chord*)(parent()); }

      virtual void read(XmlReader&);
      virtual void write(Xml& xml) const;
      virtual void layout();
      virtual void draw(QPainter*) const;

      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      };

#endif

