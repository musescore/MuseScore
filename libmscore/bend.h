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

#ifndef __BEND_H__
#define __BEND_H__

#include "element.h"
#include "pitchvalue.h"
#include "property.h"

namespace Ms {

//---------------------------------------------------------
//   @@ Bend
//---------------------------------------------------------

class Bend : public Element {
      Q_GADGET

      QString fontFace   { "FreeSerif" };
      qreal fontSize     { 8.0         };
      bool fontBold      { false       };
      bool fontItalic    { false       };
      bool fontUnderline { false       };

      PropertyFlags propertyFlagsList[5] = {
            PropertyFlags::STYLED,
            PropertyFlags::STYLED,
            PropertyFlags::STYLED,
            PropertyFlags::STYLED,
            PropertyFlags::STYLED,
            };

      bool _playBend     { true };
      QList<PitchValue> _points;
      qreal _lw;
      QPointF notePos;
      qreal noteWidth;

      QFont font(qreal) const;

   public:
      Bend(Score* s);
      virtual Bend* clone() const override        { return new Bend(*this); }
      virtual ElementType type() const override   { return ElementType::BEND; }
      virtual void layout() override;
      virtual void draw(QPainter*) const override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader& e) override;
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }
      bool playBend() const                      { return _playBend; }
      void setPlayBend(bool v)                   { _playBend = v;    }

      // property methods
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      virtual void setPropertyFlags(P_ID, PropertyFlags) override;
      virtual PropertyFlags propertyFlags(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual StyleIdx getPropertyStyle(P_ID) const override;
      virtual void reset() override;
      };


}     // namespace Ms
#endif

