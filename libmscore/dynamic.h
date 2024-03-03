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

#include "textbase.h"

namespace Ms {

class Measure;
class Segment;

//-----------------------------------------------------------------------------
//   Dyn
//    see: http://en.wikipedia.org/wiki/File:Dynamic's_Note_Velocity.svg
//-----------------------------------------------------------------------------

struct Dyn {
      int velocity;      ///< associated midi velocity (0-127, -1 = none)
      bool accent;       ///< if true add velocity to current chord velocity
      const char* tag;   // name of dynamics, eg. "fff"
      const char* text;  // utf8 text of dynamic
      int changeInVelocity;
      };

// variant with ligatures, works for both emmentaler and bravura:

static const Dyn dynList[] = {
      // dynamic:
      {  -1,  true,  "other-dynamics", "", 0 },
      {   1,  false, "pppppp", "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {   5,  false, "ppppp",  "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {  10,  false, "pppp",   "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {  16,  false, "ppp",    "<sym>dynamicPiano</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {  33,  false, "pp",     "<sym>dynamicPiano</sym><sym>dynamicPiano</sym>", 0 },
      {  49,  false, "p",      "<sym>dynamicPiano</sym>", 0 },
      {  64,  false, "mp",     "<sym>dynamicMezzo</sym><sym>dynamicPiano</sym>", 0 },
      {  80,  false, "mf",     "<sym>dynamicMezzo</sym><sym>dynamicForte</sym>", 0 },
      {  96,  false, "f",      "<sym>dynamicForte</sym>", 0 },
      { 112,  false, "ff",     "<sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
      { 126,  false, "fff",    "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
      { 127,  false, "ffff",   "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
      { 127,  false, "fffff",  "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },
      { 127,  false, "ffffff", "<sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", 0 },

      // accents:
      {  96,  true,  "fp",     "<sym>dynamicForte</sym><sym>dynamicPiano</sym>", -47 },
      {  49,  true,  "pf",     "<sym>dynamicPiano</sym><sym>dynamicForte</sym>", 47 },
      {  112, true,  "sf",     "<sym>dynamicSforzando</sym><sym>dynamicForte</sym>", -18 },
      {  112, true,  "sfz",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  126, true,  "sff",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", -18 },
      {  126, true,  "sffz",   "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  127, true,  "sfff",   "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym>", -18 },
      {  127, true,  "sfffz",  "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  112, true,  "sfp",    "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym>", -47 },
      {  112, true,  "sfpp",   "<sym>dynamicSforzando</sym><sym>dynamicForte</sym><sym>dynamicPiano</sym><sym>dynamicPiano</sym>", -79 },
      {  112, true,  "rfz",    "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  112, true,  "rf",     "<sym>dynamicRinforzando</sym><sym>dynamicForte</sym>", -18 },
      {  112, true,  "fz",     "<sym>dynamicForte</sym><sym>dynamicZ</sym>", -18 },
      {  96,  true,  "m",      "<sym>dynamicMezzo</sym>", -16 },
      {  112, true,  "r",      "<sym>dynamicRinforzando</sym>", -18 },
      {  112, true,  "s",      "<sym>dynamicSforzando</sym>", -18 },
      {  80,  true,  "z",      "<sym>dynamicZ</sym>", 0 },
      {  49,  true,  "n",      "<sym>dynamicNiente</sym>", -48 }
      };


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
            SFFF,
            SFFFZ,
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

      QRectF drag(EditData&) override;

   public:
      Dynamic(Score*);
      Dynamic(const Dynamic&);
      Dynamic* clone() const override     { return new Dynamic(*this); }
      ElementType type() const override   { return ElementType::DYNAMIC; }
      Segment* segment() const            { return (Segment*)parent(); }
      Measure* measure() const            { return (Measure*)parent()->parent(); }

      void setDynamicType(Type val)               { _dynamicType = val;   }
      void setDynamicType(const QString&);
      static QString dynamicTypeName(Dynamic::Type type);
      QString dynamicTypeName() const { return dynamicTypeName(_dynamicType); }
      Type dynamicType() const                     { return _dynamicType; }
      int subtype() const override         { return static_cast<int>(_dynamicType); }
      QString subtypeName() const override { return dynamicTypeName(); }

      void layout() override;
      void write(XmlWriter& xml) const override;
      void read(XmlReader&) override;

      bool isEditable() const override { return true; }
      void startEdit(EditData&) override;
      void endEdit(EditData&) override;
      void reset() override;

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
      static int dynamicVelocity(Dynamic::Type t);

      QVariant getProperty(Pid propertyId) const override;
      bool     setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid id) const override;
      Pid propertyId(const QStringRef& xmlName) const override;
      QString propertyUserValue(Pid) const override;

      std::unique_ptr<ElementGroup> getDragGroup(std::function<bool(const Element*)> isDragged) override;

      QString accessibleInfo() const override;
      QString screenReaderInfo() const override;
      void doAutoplace();

      static const std::vector<ChangeSpeedItem> changeSpeedTable;
      static int findInString(const QString& text, int& length, QString& type);
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Dynamic::Range);

#endif
