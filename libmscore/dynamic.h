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

#ifndef __DYNAMICS_H__
#define __DYNAMICS_H__

#include "text.h"
#include "mscore.h"

namespace Ms {

class Measure;
class Segment;

//-----------------------------------------------------------------------------
//   @@ Dynamic
///    dynamics marker; determines midi velocity
//
//   @P range  Ms::DynamicRange (STAFF, PART, SYSTEM)
//-----------------------------------------------------------------------------

class Dynamic : public Text {
      Q_OBJECT
      Q_PROPERTY(DynamicRange range READ dynRange  WRITE undoSetDynRange)

   public:
      enum class Type : char {
            OTHER,
            PPPPPP,
            PPPPP,
            PPPP,
            PPP,
            PP,
            P,
            MP,
            MF,
            F,
            FF,
            FFF,
            FFFF,
            FFFFF,
            FFFFFF,
            FP,
            SF,
            SFZ,
            SFF,
            SFFZ,
            SFP,
            SFPP,
            RFZ,
            RF,
            FZ,
            M,
            R,
            S,
            Z
            };

   private:
      Type _dynamicType;

      mutable QPointF dragOffset;
      int _velocity;          // associated midi velocity 0-127
      DynamicRange _dynRange;   // STAFF, PART, SYSTEM

      virtual QRectF drag(EditData*) override;

   public:
      Dynamic(Score*);
      Dynamic(const Dynamic&);
      virtual Dynamic* clone() const override   { return new Dynamic(*this); }
      virtual ElementType type() const override { return ElementType::DYNAMIC; }
      Segment* segment() const                  { return (Segment*)parent(); }
      Measure* measure() const                  { return (Measure*)parent()->parent(); }

      void setDynamicType(Type val)      { _dynamicType = val; }
      void setDynamicType(const QString&);
      QString dynamicTypeName() const;
      Type dynamicType() const           { return _dynamicType; }

      virtual void layout() override;
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;

      virtual bool isEditable() const override { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;
      virtual void reset() override;

      void setVelocity(int v);
      int velocity() const;
      DynamicRange dynRange() const    { return _dynRange; }
      void setDynRange(DynamicRange t) { _dynRange = t;    }
      void undoSetDynRange(DynamicRange t);

      virtual QLineF dragAnchor() const override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool     setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      };


}     // namespace Ms
#endif
