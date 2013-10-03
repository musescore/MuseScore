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

class QPainter;

namespace Ms {

class Chord;

//---------------------------------------------------------
//   @@ Stem
///   Graphic representation of a note stem.
//---------------------------------------------------------

class Stem : public Element {
      Q_OBJECT

      QLineF line;            // p1 is attached to note head
      qreal _userLen;
      qreal _len;             // allways positive

      virtual void startEdit(MuseScoreView*, const QPointF&);

   public:
      Stem(Score*);
      Stem &operator=(const Stem&);

      virtual Stem* clone() const      { return new Stem(*this); }
      virtual ElementType type() const { return STEM; }
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const  { return true; }
      virtual void layout();
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader& e);
      virtual void reset();
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);

      Chord* chord() const            { return (Chord*)parent(); }
      bool up() const;

      qreal userLen() const           { return _userLen; }
      void setUserLen(qreal l)        { _userLen = l; }

      qreal lineWidth() const;

      QPointF hookPos() const;
      void setLen(qreal l);
      qreal len() const               { return _len; }
      qreal stemLen() const;
      QPointF p2() const              { return line.p2(); }
      };


}     // namespace Ms
#endif

