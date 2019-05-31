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

class ChordLine final : public Element {
      ChordLineType _chordLineType;
      bool _straight;
      QPainterPath path;
      bool modified;
      qreal _lengthX;
      qreal _lengthY;
      const int _initialLength = 2;

   public:
      ChordLine(Score*);
      ChordLine(const ChordLine&);

      virtual ChordLine* clone() const override { return new ChordLine(*this); }
      virtual ElementType type() const override { return ElementType::CHORDLINE; }

      virtual void setChordLineType(ChordLineType);
      ChordLineType chordLineType() const       { return _chordLineType; }
      Chord* chord() const                      { return (Chord*)(parent()); }
      virtual bool isStraight() const           { return _straight; }
      virtual void setStraight(bool straight)   { _straight =  straight; }
      virtual void setLengthX(qreal length)     { _lengthX = length; }
      virtual void setLengthY(qreal length)     { _lengthY = length; }

      virtual void read(XmlReader&) override;
      virtual void write(XmlWriter& xml) const override;
      virtual void layout() override;
      virtual void draw(QPainter*) const override;

      virtual void startEdit(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;

      virtual QString accessibleInfo() const override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      virtual Pid propertyId(const QStringRef& xmlName) const override;
      };

extern const char* scorelineNames[];

}     // namespace Ms
#endif

