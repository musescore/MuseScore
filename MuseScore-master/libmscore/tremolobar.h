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

#ifndef __TREMOLOBAR_H__
#define __TREMOLOBAR_H__

#include "element.h"
#include "pitchvalue.h"

class QPainter;

namespace Ms {

//---------------------------------------------------------
//   @@ TremoloBar
//
//   @P userMag    qreal
//---------------------------------------------------------

class TremoloBar : public Element {
      Q_OBJECT

      Q_PROPERTY(qreal userMag  READ userMag    WRITE undoSetUserMag)

      QList<PitchValue> _points;
      qreal _lw;
      QPointF notePos;
      qreal noteWidth;
      qreal _userMag     { 1.0   };             // allowed 0.1 - 10.0

   public:
      TremoloBar(Score* s);
      virtual TremoloBar* clone() const  { return new TremoloBar(*this); }
      virtual Element::Type type() const { return Element::Type::TREMOLOBAR; }
      virtual void layout();
      virtual void draw(QPainter*) const;
      virtual void write(Xml&) const;
      virtual void read(XmlReader& e);
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }

      void undoSetUserMag(qreal val);

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      qreal userMag() const         { return _userMag;   }
      void setUserMag(qreal m)      { _userMag = m;      }
      };


}     // namespace Ms
#endif

