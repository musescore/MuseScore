//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STEM_H__
#define __STEM_H__

#include "element.h"

namespace Ms {

class Chord;

//---------------------------------------------------------
//   @@ Stem
///    Graphic representation of a note stem.
//---------------------------------------------------------

class Stem : public Element {
      Q_OBJECT

      QLineF line;                  // p1 is attached to notehead
      qreal _userLen   { 0.0 };
      qreal _len       { 0.0 };     // allways positive
      qreal _lineWidth;

      virtual void startEdit(MuseScoreView*, const QPointF&);

   public:
      Stem(Score* = 0);
      Stem &operator=(const Stem&) = delete;

      virtual Stem* clone() const        { return new Stem(*this); }
      virtual ElementType type() const { return ElementType::STEM; }
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const    { return true; }
      virtual void layout();
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

      virtual void editDrag(const EditData&);
      virtual void updateGrips(Grip*, QVector<QRectF>&) const;
      virtual int grips() const override { return 1; }
      virtual void write(XmlWriter& xml) const;
      virtual void read(XmlReader& e);
      virtual void reset();
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);

      Chord* chord() const            { return (Chord*)parent(); }
      bool up() const;

      qreal userLen() const           { return _userLen; }
      void setUserLen(qreal l)        { _userLen = l; }

      qreal lineWidth() const         { return _lineWidth; }

      QPointF hookPos() const;
      void setLen(qreal l);
      qreal len() const               { return _len; }
      qreal stemLen() const;
      QPointF p2() const              { return line.p2(); }
      };


}     // namespace Ms
#endif

