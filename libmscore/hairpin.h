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
#include "textlinebase.h"
#include "mscore.h"

class QPainter;

namespace Ms {

class Score;
class Hairpin;

enum class HairpinType : char { CRESC_HAIRPIN, DECRESC_HAIRPIN, CRESC_LINE, DECRESC_LINE };

//---------------------------------------------------------
//   @@ HairpinSegment
//---------------------------------------------------------

class HairpinSegment : public TextLineBaseSegment {
      Q_OBJECT

      bool drawCircledTip;
      QPointF circledTip;
      qreal circledTipRadius;

   public:
      HairpinSegment(Score* s) : TextLineBaseSegment(s) {}
      Hairpin* hairpin() const                       { return (Hairpin*)spanner(); }
      virtual HairpinSegment* clone() const override { return new HairpinSegment(*this); }
      virtual Element::Type type() const override    { return Element::Type::HAIRPIN_SEGMENT; }
      virtual void draw(QPainter*) const override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 4; }
      virtual void editDrag(const EditData&) override;
      virtual void layout() override;
      virtual QVariant getProperty(P_ID) const override;
      virtual bool setProperty(P_ID, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual StyleIdx getPropertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual Shape shape() const override;
      };

//---------------------------------------------------------
//   @@ Hairpin
//   @P dynRange     enum (Dynamic.STAFF, Dynamic.PART, Dynamic.SYSTEM)
//   @P hairpinType  enum (Hairpin.CRESCENDO, Hairpin.DECRESCENDO)
//   @P veloChange   int
//---------------------------------------------------------

class Hairpin : public TextLineBase {
      Q_OBJECT
      Q_ENUMS(Type)
      Q_ENUMS(Ms::Dynamic::Range)

   private:
      Q_PROPERTY(Ms::Dynamic::Range dynRange    READ  dynRange    WRITE undoSetDynRange)
      Q_PROPERTY(Ms::HairpinType    hairpinType READ  hairpinType WRITE undoSetHairpinType)
      Q_PROPERTY(int                veloChange  READ  veloChange  WRITE undoSetVeloChange)

      bool  _hairpinCircledTip;
      HairpinType _hairpinType;
      int _veloChange;
      Dynamic::Range _dynRange;
      PropertyStyle lineWidthStyle;

      Spatium _hairpinHeight;
      Spatium _hairpinContHeight;
      PropertyStyle hairpinHeightStyle;
      PropertyStyle hairpinContHeightStyle;

      static Spatium editHairpinHeight;
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;

   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const override     { return new Hairpin(*this); }
      virtual Element::Type type() const override { return Element::Type::HAIRPIN;  }

      HairpinType hairpinType() const      { return _hairpinType; }
      void setHairpinType(HairpinType val);
      void undoSetHairpinType(HairpinType);

      Segment* segment() const      { return (Segment*)parent(); }
      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;

      bool hairpinCircledTip() const           { return _hairpinCircledTip; }
      void setHairpinCircledTip(bool val)      { _hairpinCircledTip = val; }

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
      virtual StyleIdx getPropertyStyle(P_ID) const override;

      virtual void setYoff(qreal) override;
      virtual void styleChanged() override;
      virtual void reset() override;
      virtual QString accessibleInfo() const override;
      };

extern Dynamic* lookupDynamic(Element* e);

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::HairpinType);

#endif

