//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __HAIRPIN_H__
#define __HAIRPIN_H__

#include "element.h"
#include "line.h"
#include "mscore.h"

class QPainter;

namespace Ms {

class Score;
class Hairpin;

//---------------------------------------------------------
//   @@ HairpinSegment
//---------------------------------------------------------

class HairpinSegment : public LineSegment {
      Q_OBJECT

      QLineF l1, l2;

   protected:
   public:
      HairpinSegment(Score* s) : LineSegment(s) {}
      Hairpin* hairpin() const                  { return (Hairpin*)spanner(); }
      virtual HairpinSegment* clone() const override { return new HairpinSegment(*this); }
      virtual ElementType type() const override { return HAIRPIN_SEGMENT; }
      virtual void draw(QPainter*) const override;
      virtual void updateGrips(int*, QRectF*) const override;
      virtual void layout() override;
      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID id, const QVariant& v) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual PropertyStyle propertyStyle(P_ID id) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void reset() override { spanner()->reset(); }
      };

//---------------------------------------------------------
//   @@ Hairpin
//   @P hairpinType enum HairpinType CRESCENDO, DECRESCENDO
//   @P veloChange int
//   @P dynRange   enum Element::DynamicRange
//---------------------------------------------------------

class Hairpin : public SLine {
      Q_OBJECT
      Q_ENUMS(HairpinType)

   public:
      enum HairpinType { CRESCENDO, DECRESCENDO };

   private:
      Q_PROPERTY(HairpinType hairpinType READ hairpinType WRITE undoSetHairpinType)
      Q_PROPERTY(int         veloChange  READ veloChange  WRITE undoSetVeloChange)
      Q_PROPERTY(Ms::Element::DynamicRange   dynRange READ dynRange WRITE undoSetDynRange)

      HairpinType _hairpinType;
      int _veloChange;
      DynamicRange _dynRange;
      PropertyStyle lineWidthStyle;
      Spatium _hairpinHeight;
      PropertyStyle hairpinHeightStyle;
      Spatium _hairpinContHeight;
      PropertyStyle hairpinContHeightStyle;

   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const override   { return new Hairpin(*this); }
      virtual ElementType type() const override { return HAIRPIN;  }

      HairpinType hairpinType() const      { return _hairpinType; }
      void setHairpinType(HairpinType val) { _hairpinType = val;  }
      void undoSetHairpinType(HairpinType);

      Segment* segment() const         { return (Segment*)parent(); }
      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;

      int veloChange() const           { return _veloChange; }
      void setVeloChange(int v)        { _veloChange = v;    }
      void undoSetVeloChange(int v);

      DynamicRange dynRange() const          { return _dynRange; }
      void setDynRange(DynamicRange t)       { _dynRange = t;    }
      void undoSetDynRange(DynamicRange t);

      Spatium hairpinHeight() const          { return _hairpinHeight; }
      void setHairpinHeight(Spatium val)     { _hairpinHeight = val; }

      Spatium hairpinContHeight() const      { return _hairpinContHeight; }
      void setHairpinContHeight(Spatium val) { _hairpinContHeight = val; }

      virtual void write(Xml&) const override;
      virtual void read(XmlReader&) override;

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual PropertyStyle propertyStyle(P_ID id) const override;
      virtual void resetProperty(P_ID id) override;

      virtual void setYoff(qreal) override;
      virtual void styleChanged() override;
      virtual void reset() override;
      };

}     // namespace Ms
#endif

