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
///   dynamics marker; determines midi velocity
//
//    @P dynRange  DynamicRange  DYNAMIC_STAFF, DYNAMIC_PART, DYNAMIC_SYSTEM
//-----------------------------------------------------------------------------

class Dynamic : public Text {
      Q_OBJECT
      Q_PROPERTY(DynamicRange type READ dynRange  WRITE undoSetDynRange)

   public:
      enum DynamicType {
            DYNAMIC_OTHER,
            DYNAMIC_pppppp,
            DYNAMIC_ppppp,
            DYNAMIC_pppp,
            DYNAMIC_ppp,
            DYNAMIC_pp,
            DYNAMIC_p,
            DYNAMIC_mp,
            DYNAMIC_mf,
            DYNAMIC_f,
            DYNAMIC_ff,
            DYNAMIC_fff,
            DYNAMIC_ffff,
            DYNAMIC_fffff,
            DYNAMIC_ffffff,
            DYNAMIC_fp,
            DYNAMIC_sf,
            DYNAMIC_sfz,
            DYNAMIC_sff,
            DYNAMIC_sffz,
            DYNAMIC_sfp,
            DYNAMIC_sfpp,
            DYNAMIC_rfz,
            DYNAMIC_rf,
            DYNAMIC_fz,
            DYNAMIC_m,
            DYNAMIC_r,
            DYNAMIC_s,
            DYNAMIC_z
            };

   private:
      DynamicType _dynamicType;

      mutable QPointF dragOffset;
      int _velocity;          // associated midi velocity 0-127
      DynamicRange _dynRange;   // DYNAMIC_STAFF, DYNAMIC_PART, DYNAMIC_SYSTEM

      virtual QRectF drag(EditData*) override;

   public:
      Dynamic(Score*);
      Dynamic(const Dynamic&);
      virtual Dynamic* clone() const   { return new Dynamic(*this); }
      virtual ElementType type() const { return DYNAMIC; }
      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      void setDynamicType(DynamicType val) { _dynamicType = val; }
      void setDynamicType(const QString&);
      QString dynamicTypeName() const;
      DynamicType dynamicType() const      { return _dynamicType; }

      virtual void layout();
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);

      virtual bool isEditable() const { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void endEdit();
      virtual void reset();

      void setVelocity(int v);
      int velocity() const;
      DynamicRange dynRange() const    { return _dynRange; }
      void setDynRange(DynamicRange t) { _dynRange = t;    }
      void undoSetDynRange(DynamicRange t);

      virtual QLineF dragAnchor() const;

      QVariant getProperty(P_ID propertyId) const;
      bool     setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };


}     // namespace Ms
#endif
