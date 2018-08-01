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

#include "textlinebase.h"

namespace Ms {

class Score;
class XmlWriter;
class Volta;
class Measure;

extern void vdebug(int n);
extern LineSegment* voltaDebug;

//---------------------------------------------------------
//   @@ VoltaSegment
//---------------------------------------------------------

class VoltaSegment final : public TextLineBaseSegment {
   public:
      VoltaSegment(Score* s) : TextLineBaseSegment(s, ElementFlag::MOVABLE | ElementFlag::ON_STAFF) {}
      virtual ElementType type() const override     { return ElementType::VOLTA_SEGMENT; }
      virtual VoltaSegment* clone() const override  { return new VoltaSegment(*this); }
      Volta* volta() const                          { return (Volta*)spanner(); }
      virtual void layout() override;

      virtual Element* propertyDelegate(Pid) override;
      };

//---------------------------------------------------------
//   @@ Volta
//   @P voltaType  enum (Volta.CLOSE, Volta.OPEN)
//---------------------------------------------------------

class Volta final : public TextLineBase {
      QList<int> _endings;

   public:
      enum class Type : char {
            OPEN, CLOSED
            };

      Volta(Score* s);
      virtual Volta* clone()       const override { return new Volta(*this); }
      virtual ElementType type() const override   { return ElementType::VOLTA; }
      virtual LineSegment* createLineSegment() override;

      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader& e) override;

      QList<int> endings() const           { return _endings; }
      QList<int>& endings()                { return _endings; }
      void setEndings(const QList<int>& l) { _endings = l;    }
      void setText(const QString& s);
      QString text() const;

      bool hasEnding(int repeat) const;
      int lastEnding() const;
      void setVoltaType(Volta::Type);     // deprecated
      Type voltaType() const;             // deprecated

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;

      virtual QString accessibleInfo() const override;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Volta::Type);

#endif

