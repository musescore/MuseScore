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
      virtual void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;

   public:
      OttavaSegment(Score* s) : TextLineBaseSegment(s, ElementFlag::MOVABLE)  { }
      virtual ElementType type() const override     { return ElementType::OTTAVA_SEGMENT; }
      virtual OttavaSegment* clone() const override { return new OttavaSegment(*this); }
      Ottava* ottava() const                        { return (Ottava*)spanner(); }
      virtual void layout() override;
      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      };

//---------------------------------------------------------
//   @@ Ottava
//   @P ottavaType  enum (Ottava.OTTAVA_8VA, .OTTAVA_8VB, .OTTAVA_15MA, .OTTAVA_15MB, .OTTAVA_22MA, .OTTAVA_22MB)
//---------------------------------------------------------

class Ottava final : public TextLineBase {
      std::vector<StyledProperty> _styledProperties;
      OttavaType _ottavaType;
      bool _numbersOnly;

      void updateStyledProperties();

   protected:
      friend class OttavaSegment;

   public:
      Ottava(Score* s);
      Ottava(const Ottava&);
      virtual Ottava* clone() const override    { return new Ottava(*this); }
      virtual ElementType type() const override { return ElementType::OTTAVA; }

      virtual const StyledProperty* styledProperties() const override { return _styledProperties.data(); }
      StyledProperty* styledProperties() { return _styledProperties.data(); }

      void setOttavaType(OttavaType val);
      OttavaType ottavaType() const             { return _ottavaType; }

      bool numbersOnly() const                  { return _numbersOnly; }
      void setNumbersOnly(bool val);

      void setPlacement(Placement);

      virtual LineSegment* createLineSegment() override;
      int pitchShift() const;

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader& de) override;
      bool readProperties(XmlReader& e);

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;

      virtual void setYoff(qreal) override;

      virtual QString accessibleInfo() const override;
      };

}     // namespace Ms

#endif

