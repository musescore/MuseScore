//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SPACER_H__
#define __SPACER_H__

#include "element.h"

class QPainter;

namespace Ms {

//---------------------------------------------------------
//   SpacerType
//---------------------------------------------------------

enum class SpacerType : char {
      UP, DOWN, FIXED
      };

//-------------------------------------------------------------------
//   @@ Spacer
///    Vertical spacer element to adjust the distance of staves.
//-------------------------------------------------------------------

class Spacer : public Element {
      Q_OBJECT

      SpacerType _spacerType;
      qreal _gap;

      QPainterPath path;

      void layout0();

   public:
      Spacer(Score*);
      Spacer(const Spacer&);
      virtual Spacer* clone() const    { return new Spacer(*this); }
      virtual Element::Type type() const { return Element::Type::SPACER; }
      SpacerType spacerType() const    { return _spacerType; }
      void setSpacerType(SpacerType t) { _spacerType = t; }

      virtual void write(Xml&) const;
      virtual void read(XmlReader&);
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 1; }
      virtual void spatiumChanged(qreal, qreal);
      void setGap(qreal sp);
      qreal gap() const     { return _gap; }

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };


}     // namespace Ms
#endif
