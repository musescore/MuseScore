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

enum class ChangeMethod : signed char;

enum class HairpinType : signed char {
      INVALID = -1,
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

      void startEditDrag(EditData&) override;
      void editDrag(EditData&) override;

      void draw(QPainter*) const override;
      Sid getPropertyStyle(Pid) const override;

      bool acceptDrop(EditData&) const override;
      Element* drop(EditData&) override;

   public:
      HairpinSegment(Spanner* sp, Score* s);

      HairpinSegment* clone() const override { return new HairpinSegment(*this);    }
      ElementType type() const override      { return ElementType::HAIRPIN_SEGMENT; }

      Hairpin* hairpin() const                       { return (Hairpin*)spanner();          }

      Element* propertyDelegate(Pid) override;

      void layout() override;
      Shape shape() const override;

      int gripsCount() const override { return 4; }
      std::vector<QPointF> gripsPositions(const EditData& = EditData()) const override;

      std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const Element*)> isDragged) override;
      };

//---------------------------------------------------------
//   @@ Hairpin
//   @P dynRange     enum (Dynamic.STAFF, Dynamic.PART, Dynamic.SYSTEM)
//   @P hairpinType  enum (Hairpin.CRESCENDO, Hairpin.DECRESCENDO)
//   @P veloChange   int
//---------------------------------------------------------

class Hairpin final : public TextLineBase {
      HairpinType _hairpinType { HairpinType::INVALID };
      int _veloChange;
      bool  _hairpinCircledTip;
      Dynamic::Range _dynRange;
      bool _singleNoteDynamics;
      ChangeMethod _veloChangeMethod;

      Spatium _hairpinHeight;
      Spatium _hairpinContHeight;

      Sid getPropertyStyle(Pid) const override;

   public:
      Hairpin(Score* s);

      Hairpin* clone() const override   { return new Hairpin(*this); }
      ElementType type() const override { return ElementType::HAIRPIN;  }

      HairpinType hairpinType() const           { return _hairpinType; }
      void setHairpinType(HairpinType val);

      Segment* segment() const                  { return (Segment*)parent(); }
      void layout() override;
      LineSegment* createLineSegment() override;

      bool hairpinCircledTip() const            { return _hairpinCircledTip; }
      void setHairpinCircledTip(bool val)       { _hairpinCircledTip = val; }

      int veloChange() const                    { return _veloChange; }
      void setVeloChange(int v)                 { _veloChange = v;    }

      Dynamic::Range dynRange() const           { return _dynRange; }
      void setDynRange(Dynamic::Range t)        { _dynRange = t;    }

      Spatium hairpinHeight() const             { return _hairpinHeight; }
      void setHairpinHeight(Spatium val)        { _hairpinHeight = val; }

      Spatium hairpinContHeight() const         { return _hairpinContHeight; }
      void setHairpinContHeight(Spatium val)    { _hairpinContHeight = val; }

      bool singleNoteDynamics() const           { return _singleNoteDynamics; }
      void setSingleNoteDynamics(bool val)      { _singleNoteDynamics = val; }

      ChangeMethod veloChangeMethod() const     { return _veloChangeMethod; }
      void setVeloChangeMethod(ChangeMethod val){ _veloChangeMethod = val; }

      bool isCrescendo() const   { return _hairpinType == HairpinType::CRESC_HAIRPIN || _hairpinType == HairpinType::CRESC_LINE; }
      bool isDecrescendo() const { return _hairpinType == HairpinType::DECRESC_HAIRPIN || _hairpinType == HairpinType::DECRESC_LINE; }

      void write(XmlWriter&) const override;
      void read(XmlReader&) override;

      QVariant getProperty(Pid id) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid id) const override;
      Pid propertyId(const QStringRef& xmlName) const override;

      QString accessibleInfo() const override;
      bool isLineType() const  { return _hairpinType == HairpinType::CRESC_LINE || _hairpinType == HairpinType::DECRESC_LINE; }
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::HairpinType);

#endif

