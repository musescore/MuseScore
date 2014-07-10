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

class QPainter;

namespace Ms {

class Chord;

// subtypes:
enum class ChordLineType : char {
      NOTYPE, FALL, DOIT,
      PLOP, SCOOP
      };

//---------------------------------------------------------
//   @@ ChordLine
///    bezier line attached to top note of a chord
///    implements fall, doit, plop, bend
//---------------------------------------------------------

class ChordLine : public Element {
      Q_OBJECT

      ChordLineType _chordLineType;
      bool _straight;
      QPainterPath path;
      bool modified;
      float _lengthX;
      float _lengthY;

   public:
      ChordLine(Score*);
      ChordLine(const ChordLine&);

      virtual ChordLine* clone() const    { return new ChordLine(*this); }
      virtual Element::Type type() const  { return Element::Type::CHORDLINE; }
      virtual void setChordLineType(ChordLineType);
      ChordLineType chordLineType() const { return _chordLineType; }
      Chord* chord() const                { return (Chord*)(parent()); }
      virtual bool isStraight() const     { return _straight; }
      virtual void setStraight(bool straight)   { _straight =  straight; }
      virtual void setLengthX(float length)     { _lengthX = length; }
      virtual void setLengthY(float length)     { _lengthY = length; }

      virtual void read(XmlReader&);
      virtual void write(Xml& xml) const;
      virtual void layout();
      virtual void draw(QPainter*) const;

      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, int*, QRectF*) const override;

      virtual QString accessibleInfo() override;
      };

extern const char* scorelineNames[];

}     // namespace Ms
#endif

