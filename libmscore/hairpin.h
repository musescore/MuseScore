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
#include "dynamic.h"
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
      bool drawCircledTip;
      QPointF circledTip;
      qreal circledTipRadius;

   protected:
   public:
      HairpinSegment(Score* s) : LineSegment(s) {}
      Hairpin* hairpin() const                       { return (Hairpin*)spanner(); }
      virtual HairpinSegment* clone() const override { return new HairpinSegment(*this); }
      virtual Element::Type type() const override    { return Element::Type::HAIRPIN_SEGMENT; }
      virtual void draw(QPainter*) const override;
      virtual void updateGrips(int*, int*, QRectF*) const override;
      virtual void editDrag(const EditData&) override;
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
//   @P hairpinType  Ms::Hairpin::Type  (CRESCENDO, DECRESCENDO)
//   @P veloChange   int
//   @P dynRange     Ms::Dynamic::Range (STAFF, PART, SYSTEM)
//---------------------------------------------------------

class Hairpin : public SLine {
      Q_OBJECT
      Q_ENUMS(Type)
      Q_ENUMS(Ms::Dynamic::Range)

   public:
      enum class Type : char { CRESCENDO, DECRESCENDO };

   private:
      Q_PROPERTY(Ms::Hairpin::Type  hairpinType READ  hairpinType WRITE undoSetHairpinType)
      Q_PROPERTY(int                veloChange  READ  veloChange  WRITE undoSetVeloChange)
      Q_PROPERTY(Ms::Dynamic::Range dynRange    READ  dynRange    WRITE undoSetDynRange)

      bool  _hairpinCircledTip;
      Type _hairpinType;
      int _veloChange;
      Dynamic::Range _dynRange;
      PropertyStyle lineWidthStyle;

      Spatium _hairpinHeight;
      PropertyStyle hairpinHeightStyle;
      Spatium _hairpinContHeight;
      PropertyStyle hairpinContHeightStyle;


   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const override     { return new Hairpin(*this); }
      virtual Element::Type type() const override { return Element::Type::HAIRPIN;  }

      Type hairpinType() const      { return _hairpinType; }
      void setHairpinType(Type val) { _hairpinType = val;  }
      void undoSetHairpinType(Type);

      Segment* segment() const      { return (Segment*)parent(); }
      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;

      bool hairpinCircledTip() const           { return _hairpinCircledTip; }
      void setHairpinCircledTip( bool val )    { _hairpinCircledTip = val; }

      int veloChange() const           { return _veloChange; }
      void setVeloChange(int v)        { _veloChange = v;    }
      void undoSetVeloChange(int v);

      Dynamic::Range dynRange() const        { return _dynRange; }
      void setDynRange(Dynamic::Range t)     { _dynRange = t;    }
      void undoSetDynRange(Dynamic::Range t);

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
      virtual QString accessibleInfo() override;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Hairpin::Type);

#endif

