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
      VoltaSegment(Spanner*, Score*);

      ElementType type() const override     { return ElementType::VOLTA_SEGMENT; }
      VoltaSegment* clone() const override  { return new VoltaSegment(*this); }

      Volta* volta() const                  { return (Volta*)spanner(); }
      void layout() override;

      Element* propertyDelegate(Pid) override;
      };

//---------------------------------------------------------
//   @@ Volta
//   @P voltaType  enum (Volta.CLOSE, Volta.OPEN)
//---------------------------------------------------------

class Volta final : public TextLineBase {
      QList<int> _endings;
      static constexpr Anchor VOLTA_ANCHOR = Anchor::MEASURE;

   public:
      enum class Type : char {
            OPEN, CLOSED
            };

      Volta(Score* s);

      Volta* clone() const override       { return new Volta(*this); }
      ElementType type() const override   { return ElementType::VOLTA; }

      LineSegment* createLineSegment() override;

      void write(XmlWriter&) const override;
      void read(XmlReader& e) override;

      bool readProperties(XmlReader&) override;
      SpannerSegment* layoutSystem(System* system) override;

      void setVelocity() const;
      void setChannel() const;
      void setTempo() const;

      QList<int> endings() const           { return _endings; }
      QList<int>& endings()                { return _endings; }
      void setEndings(const QList<int>& l);
      void setText(const QString& s);
      QString text() const;

      bool hasEnding(int repeat) const;
      int firstEnding() const;
      int lastEnding() const;
      void setVoltaType(Volta::Type);     // deprecated
      Type voltaType() const;             // deprecated

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

      QString accessibleInfo() const override;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Volta::Type);

#endif

