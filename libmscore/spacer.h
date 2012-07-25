//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: spacer.h 5656 2012-05-21 15:36:47Z wschweer $
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

//---------------------------------------------------------
//   SpacerType
//---------------------------------------------------------

enum SpacerType {
      SPACER_UP, SPACER_DOWN
      };

//-------------------------------------------------------------------
//   @@ Spacer
//    Vertical spacer element to adjust the distance of staves.
//-------------------------------------------------------------------

class Spacer : public Element {
      Q_OBJECT

      SpacerType _subtype;
      qreal _gap;

      QPainterPath path;

      void layout0();

   public:
      Spacer(Score*);
      Spacer(const Spacer&);
      virtual Spacer* clone() const    { return new Spacer(*this); }
      virtual ElementType type() const { return SPACER; }
      SpacerType subtype() const { return _subtype; }
      void subtype(SpacerType t) { _subtype = t;    }
      void setSubtype(SpacerType val);

      virtual void write(Xml&) const;
      virtual void read(const QDomElement&);
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual void spatiumChanged(qreal, qreal);
      void setGap(qreal sp);
      qreal gap() const     { return _gap; }

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };

#endif
