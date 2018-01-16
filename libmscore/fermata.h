//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __FERMATA_H__
#define __FERMATA_H__

#include "element.h"
#include "mscore.h"

namespace Ms {

class ChordRest;
class Segment;
class Measure;
class System;
class Page;

enum class SymId;

//---------------------------------------------------------
//    Fermata
//---------------------------------------------------------

class Fermata final : public Element {
      SymId _symId;
      qreal _timeStretch;
      bool _play;

      virtual void draw(QPainter*) const override;

   public:
      Fermata(Score*);
      Fermata(SymId, Score*);
      Fermata &operator=(const Fermata&) = delete;

      virtual Fermata* clone() const override    { return new Fermata(*this); }
      virtual ElementType type() const override  { return ElementType::FERMATA; }

      virtual qreal mag() const override;

      SymId symId() const                   { return _symId; }
      void setSymId(SymId id)               { _symId  = id;  }
      virtual int subtype() const override  { return int(_symId); }
      QString userName() const;

      virtual void layout() override;

      virtual void read(XmlReader&) override;
      virtual void write(XmlWriter& xml) const override;
      virtual bool readProperties(XmlReader&) override;

      virtual void reset() override;
      virtual QLineF dragAnchor() const override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyFlags& propertyFlags(P_ID) override;
      virtual void resetProperty(P_ID id) override;
      StyleIdx getPropertyStyle(P_ID id) const override;

      ChordRest* chordRest() const;
      Segment* segment() const              { return toSegment(parent()); }
      Measure* measure() const;
      System* system() const;
      Page* page() const;

      qreal timeStretch() const             { return _timeStretch; }
      void setTimeStretch(qreal val)        { _timeStretch = val;  }

      bool play() const                     { return _play; }
      void setPlay(bool val)                { _play = val;  }

      QString accessibleInfo() const override;
      };

}     // namespace Ms
#endif

