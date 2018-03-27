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

namespace Ms {

class Score;
class Hairpin;

enum class HairpinType : char {
      CRESC_HAIRPIN,
      DECRESC_HAIRPIN,
      CRESC_LINE,
      DECRESC_LINE
      };

//---------------------------------------------------------
//   @@ HairpinSegment
//---------------------------------------------------------

class HairpinSegment final : public TextLineBaseSegment {
      bool    drawCircledTip;
      QPointF circledTip;
      qreal   circledTipRadius;

      virtual void startEdit(EditData&) override;
      virtual void startEditDrag(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;

      virtual void draw(QPainter*) const override;

   public:
      HairpinSegment(Score* s);
      virtual HairpinSegment* clone() const override { return new HairpinSegment(*this);    }
      virtual ElementType type() const override      { return ElementType::HAIRPIN_SEGMENT; }

      Hairpin* hairpin() const                       { return (Hairpin*)spanner();          }

      virtual QVariant getProperty(P_ID) const override;
      virtual bool setProperty(P_ID, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      virtual void layout() override;
      virtual Shape shape() const override;
      };

//---------------------------------------------------------
//   @@ Hairpin
//   @P dynRange     enum (Dynamic.STAFF, Dynamic.PART, Dynamic.SYSTEM)
//   @P hairpinType  enum (Hairpin.CRESCENDO, Hairpin.DECRESCENDO)
//   @P veloChange   int
//---------------------------------------------------------

class Hairpin final : public TextLineBase {
      HairpinType _hairpinType;
      int _veloChange;
      bool  _hairpinCircledTip;
      Dynamic::Range _dynRange;

      Spatium _hairpinHeight;
      Spatium _hairpinContHeight;

   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const override   { return new Hairpin(*this); }
      virtual ElementType type() const override { return ElementType::HAIRPIN;  }

      HairpinType hairpinType() const           { return _hairpinType; }
      void setHairpinType(HairpinType val);
      void undoSetHairpinType(HairpinType);

      Segment* segment() const                  { return (Segment*)parent(); }
      virtual void layout() override;
      virtual LineSegment* createLineSegment() override;

      bool hairpinCircledTip() const            { return _hairpinCircledTip; }
      void setHairpinCircledTip(bool val)       { _hairpinCircledTip = val; }

      int veloChange() const                    { return _veloChange; }
      void setVeloChange(int v)                 { _veloChange = v;    }
      void undoSetVeloChange(int v);

      Dynamic::Range dynRange() const           { return _dynRange; }
      void setDynRange(Dynamic::Range t)        { _dynRange = t;    }
      void undoSetDynRange(Dynamic::Range t);

      Spatium hairpinHeight() const             { return _hairpinHeight; }
      void setHairpinHeight(Spatium val)        { _hairpinHeight = val; }

      Spatium hairpinContHeight() const         { return _hairpinContHeight; }
      void setHairpinContHeight(Spatium val)    { _hairpinContHeight = val; }

      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;

      virtual void setYoff(qreal) override;
      virtual QString accessibleInfo() const override;
      };

extern Dynamic* lookupDynamic(Element* e);

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::HairpinType);

#endif

