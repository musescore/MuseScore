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

#ifndef __VOLTA_H__
#define __VOLTA_H__

#include "textline.h"

namespace Ms {

class Score;
class Xml;
class Volta;
class Measure;

//---------------------------------------------------------
//   VoltaType
//---------------------------------------------------------

enum class VoltaType : char {
      OPEN, CLOSED
      };

extern void vdebug(int n);
extern LineSegment* voltaDebug;

//---------------------------------------------------------
//   @@ VoltaSegment
//---------------------------------------------------------

class VoltaSegment : public TextLineSegment {
      Q_OBJECT

   public:
      VoltaSegment(Score* s) : TextLineSegment(s) {}
      virtual Element::Type type() const override   { return Element::Type::VOLTA_SEGMENT; }
      virtual VoltaSegment* clone() const override  { return new VoltaSegment(*this); }
      Volta* volta() const                          { return (Volta*)spanner(); }
      virtual void layout() override;
      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;
      virtual void reset() override { spanner()->reset(); }
      };

//---------------------------------------------------------
//   @@ Volta
//   @P voltaType  Ms::VoltaType  (OPEN, CLOSED)
//---------------------------------------------------------

class Volta : public TextLine {
      Q_OBJECT

   private:
      Q_PROPERTY(VoltaType voltaType READ voltaType WRITE undoSetVoltaType)

      QList<int> _endings;
      PropertyStyle lineWidthStyle;

   public:
      Volta(Score* s);
      virtual Volta* clone()       const override { return new Volta(*this); }
      virtual Element::Type type() const override { return Element::Type::VOLTA; }
      virtual LineSegment* createLineSegment() override;
      virtual void layout() override;

      virtual void write(Xml&) const override;
      virtual void read(XmlReader& e) override;

      QList<int> endings() const           { return _endings; }
      QList<int>& endings()                { return _endings; }
      void setEndings(const QList<int>& l) { _endings = l;    }
      void setText(const QString& s);
      QString text() const;

      void setVoltaType(VoltaType val);
      void undoSetVoltaType(VoltaType val);
      VoltaType voltaType() const;

      bool hasEnding(int repeat) const;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;

      virtual void setYoff(qreal);
      virtual void reset() override;
      virtual bool systemFlag() const override  { return true;  }
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::VoltaType);

#endif

