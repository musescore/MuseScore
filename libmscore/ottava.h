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

#ifndef __OTTAVA_H__
#define __OTTAVA_H__

#include "textlinebase.h"
#include "property.h"

namespace Ms {

//---------------------------------------------------------
//   OttavaE
//---------------------------------------------------------

struct OttavaE {
      int offset;
      unsigned start;
      unsigned end;
      };

//---------------------------------------------------------
//   OttavaType
//---------------------------------------------------------

enum class OttavaType : char {
      OTTAVA_8VA,
      OTTAVA_8VB,
      OTTAVA_15MA,
      OTTAVA_15MB,
      OTTAVA_22MA,
      OTTAVA_22MB
      };

class Ottava;

//---------------------------------------------------------
//   @@ OttavaSegment
//---------------------------------------------------------

class OttavaSegment final : public TextLineBaseSegment {
   public:
      OttavaSegment(Score* s) : TextLineBaseSegment(s)  { }
      virtual ElementType type() const override     { return ElementType::OTTAVA_SEGMENT; }
      virtual OttavaSegment* clone() const override { return new OttavaSegment(*this); }
      Ottava* ottava() const                        { return (Ottava*)spanner(); }
      virtual void layout() override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      };

//---------------------------------------------------------
//   @@ Ottava
//   @P ottavaType  enum (Ottava.OTTAVA_8VA, .OTTAVA_8VB, .OTTAVA_15MA, .OTTAVA_15MB, .OTTAVA_22MA, .OTTAVA_22MB)
//---------------------------------------------------------

class Ottava final : public TextLineBase {
      OttavaType _ottavaType;
      bool _numbersOnly;

      int _pitchShift;

   protected:
      friend class OttavaSegment;

   public:
      Ottava(Score* s);
      Ottava(const Ottava&);
      virtual Ottava* clone() const override    { return new Ottava(*this); }
      virtual ElementType type() const override { return ElementType::OTTAVA; }

      void setOttavaType(OttavaType val);
      OttavaType ottavaType() const             { return _ottavaType; }
      void undoSetOttavaType(OttavaType val);

      bool numbersOnly() const                  { return _numbersOnly; }
      void setNumbersOnly(bool val)             { _numbersOnly = val; }

      virtual LineSegment* createLineSegment() override;
      int pitchShift() const                    { return _pitchShift; }

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader& de) override;
      bool readProperties(XmlReader& e);

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      virtual void setYoff(qreal) override;

      virtual QString accessibleInfo() const override;
      };

}     // namespace Ms

#endif

