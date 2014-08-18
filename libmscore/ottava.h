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

#include "textline.h"

namespace Ms {

//---------------------------------------------------------
//   OttavaE
//---------------------------------------------------------

struct OttavaE {
      int offset;
      unsigned start;
      unsigned end;
      };

class Ottava;

//---------------------------------------------------------
//   @@ OttavaSegment
//---------------------------------------------------------

class OttavaSegment : public TextLineSegment {
      Q_OBJECT

   protected:

   public:
      OttavaSegment(Score* s) : TextLineSegment(s)  { }
      virtual Element::Type type() const override   { return Element::Type::OTTAVA_SEGMENT; }
      virtual OttavaSegment* clone() const override { return new OttavaSegment(*this); }
      Ottava* ottava() const                        { return (Ottava*)spanner(); }
      virtual void layout() override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;
      };

//---------------------------------------------------------
//   @@ Ottava
//   @P ottavaType  Ms::Ottava::Type  (OTTAVA_8VA, OTTAVA_15MA, OTTAVA_8VB, OTTAVA_15MB, OTTAVA_22MA, OTTAVA_22MB)
//---------------------------------------------------------

class Ottava : public TextLine {
      Q_OBJECT
      Q_PROPERTY(Ms::Ottava::Type ottavaType READ ottavaType WRITE undoSetOttavaType)
      Q_ENUMS(Type)

   public:
      enum class Type : char {
            OTTAVA_8VA,
            OTTAVA_8VB,
            OTTAVA_15MA,
            OTTAVA_15MB,
            OTTAVA_22MA,
            OTTAVA_22MB
            };

   private:
      Type _ottavaType;
      bool _numbersOnly;
      PropertyStyle numbersOnlyStyle;
      PropertyStyle lineWidthStyle;
      PropertyStyle lineStyleStyle;
      PropertyStyle beginTextStyle;
      PropertyStyle continueTextStyle;

      int _pitchShift;

   protected:
      friend class OttavaSegment;

   public:
      Ottava(Score* s);
      Ottava(const Ottava&);
      virtual Ottava* clone() const override      { return new Ottava(*this); }
      virtual Element::Type type() const override { return Element::Type::OTTAVA; }

      void setOttavaType(Type val);
      Type ottavaType() const       { return _ottavaType; }
      void undoSetOttavaType(Type val);

      bool numbersOnly() const      { return _numbersOnly; }
      void setNumbersOnly(bool val) { _numbersOnly = val; }

      virtual LineSegment* createLineSegment() override;
      int pitchShift() const { return _pitchShift; }

      virtual void endEdit() override;
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader& de) override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;

      virtual void setYoff(qreal) override;
      virtual void styleChanged() override;
      virtual void reset() override;

      virtual QString accessibleInfo() override;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Ottava::Type);

#endif

