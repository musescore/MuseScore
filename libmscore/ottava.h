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
      // OttavaSegment(Score* s) : TextLineSegment(s) { setFlag(ELEMENT_ON_STAFF, true); }
      OttavaSegment(Score* s) : TextLineSegment(s) { }
      virtual ElementType type() const override     { return OTTAVA_SEGMENT; }
      virtual OttavaSegment* clone() const override { return new OttavaSegment(*this); }
      Ottava* ottava() const               { return (Ottava*)spanner(); }
      virtual void layout() override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;
      };

//---------------------------------------------------------
//   OttavaType
//---------------------------------------------------------

enum class OttavaType {
      OTTAVA_8VA,
      OTTAVA_8VB,
      OTTAVA_15MA,
      OTTAVA_15MB,
      OTTAVA_22MA,
      OTTAVA_22MB
      };

//---------------------------------------------------------
//   @@ Ottava
//   @P ottavaType   enum OttavaType OTTAVA_8VA, OTTAVA_15MA, OTTAVA_8VB, OTTAVA_15MB, OTTAVA_22MA, OTTAVA_22MB
//---------------------------------------------------------

class Ottava : public TextLine {
      Q_OBJECT
      Q_ENUMS(OttavaType)

   public:

   private:
      Q_PROPERTY(OttavaType ottavaType READ ottavaType WRITE undoSetOttavaType)
      OttavaType _ottavaType;
      bool _numbersOnly;
      PropertyStyle numbersOnlyStyle;
      PropertyStyle lineWidthStyle;
      PropertyStyle lineStyleStyle;
      PropertyStyle beginSymbolStyle;
      PropertyStyle continueSymbolStyle;

   protected:
      QString text;
      int _pitchShift;
      mutable qreal textHeight;     ///< cached value

      friend class OttavaSegment;

   public:
      Ottava(Score* s);
      virtual Ottava* clone() const override { return new Ottava(*this); }
      virtual ElementType type() const override { return OTTAVA; }

      void setOttavaType(OttavaType val);
      OttavaType ottavaType() const { return _ottavaType; }
      void undoSetOttavaType(OttavaType val);

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
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::OttavaType)

#endif

