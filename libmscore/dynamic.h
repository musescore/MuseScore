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
//   @P range  enum (Dynamic.STAFF, .PART, .SYSTEM)
//-----------------------------------------------------------------------------

class Dynamic final : public TextBase {
   Q_GADGET
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

      enum class Range : char {
            STAFF, PART, SYSTEM
            };

      enum class Speed : char {
            SLOW, NORMAL, FAST
            };

      struct ChangeSpeedItem {
            Speed speed;
            const char* name;
            };

      Q_ENUM(Type);

   private:
      Type _dynamicType;

      mutable QPointF dragOffset;
      int _velocity;     // associated midi velocity 0-127
      Range _dynRange;   // STAFF, PART, SYSTEM

      int _changeInVelocity         { 128 };
      Speed _velChangeSpeed         { Speed::NORMAL };

      virtual QRectF drag(EditData&) override;
      virtual Sid getPropertyStyle(Pid) const override;

   public:
      Dynamic(Score*);
      Dynamic(const Dynamic&);
      virtual Dynamic* clone() const override     { return new Dynamic(*this); }
      virtual ElementType type() const override   { return ElementType::DYNAMIC; }
      Segment* segment() const                    { return (Segment*)parent(); }
      Measure* measure() const                    { return (Measure*)parent()->parent(); }

      void setDynamicType(Type val)               { _dynamicType = val;   }
      void setDynamicType(const QString&);
      static QString dynamicTypeName(Dynamic::Type type);
      QString dynamicTypeName() const { return dynamicTypeName(_dynamicType); }
      Type dynamicType() const                     { return _dynamicType; }
      virtual int subtype() const override         { return (int) _dynamicType; }
      virtual QString subtypeName() const override { return dynamicTypeName(); }

      virtual void layout() override;
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;

      virtual bool isEditable() const override { return true; }
      virtual void startEdit(EditData&) override;
      virtual void endEdit(EditData&) override;
      virtual void reset() override;

      void setVelocity(int v)   { _velocity = v;    }
      int velocity() const;
      Range dynRange() const    { return _dynRange; }
      void setDynRange(Range t) { _dynRange = t;    }
      void undoSetDynRange(Range t);

      int changeInVelocity() const;
      void setChangeInVelocity(int val);
      Fraction velocityChangeLength() const;

      Speed velChangeSpeed() const  { return _velChangeSpeed; }
      void setVelChangeSpeed(Speed val) { _velChangeSpeed = val; }
      static QString speedToName(Speed speed);
      static Speed nameToSpeed(QString name);

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool     setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid id) const override;
      virtual Pid propertyId(const QStringRef& xmlName) const override;
      virtual QString propertyUserValue(Pid) const override;

      virtual QString accessibleInfo() const override;
      virtual QString screenReaderInfo() const override;
      void doAutoplace();

      static const std::vector<ChangeSpeedItem> changeSpeedTable;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Dynamic::Range);

#endif
