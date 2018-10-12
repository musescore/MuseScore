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

#ifndef __ARTICULATION_H__
#define __ARTICULATION_H__

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
//   ArticulationInfo
//    gives infos about note attributes
//---------------------------------------------------------

enum class ArticulationAnchor : char {
      TOP_STAFF,      // anchor is always placed at top of staff
      BOTTOM_STAFF,   // anchor is always placed at bottom of staff
      CHORD,          // anchor depends on chord direction, away from stem
      TOP_CHORD,      // attribute is always placed at top of chord
      BOTTOM_CHORD,   // attribute is placed at bottom of chord
      };

// flags:
enum class ArticulationShowIn : char { PITCHED_STAFF = 1, TABLATURE = 2 };

constexpr ArticulationShowIn operator| (ArticulationShowIn a1, ArticulationShowIn a2) {
      return static_cast<ArticulationShowIn>(static_cast<unsigned char>(a1) | static_cast<unsigned char>(a2));
      }
constexpr bool operator& (ArticulationShowIn a1, ArticulationShowIn a2) {
      return static_cast<unsigned char>(a1) & static_cast<unsigned char>(a2);
      }

//---------------------------------------------------------
//   @@ Articulation
///    articulation marks
//---------------------------------------------------------

class Articulation final : public Element {
      SymId _symId;
      Direction _direction;
      QString _channelName;

      ArticulationAnchor _anchor;

      bool _up;
      MScore::OrnamentStyle _ornamentStyle;     // for use in ornaments such as trill
      bool _playArticulation;

      virtual void draw(QPainter*) const;

   public:
      Articulation(Score*);
      Articulation(SymId, Score*);
      Articulation &operator=(const Articulation&) = delete;

      virtual Articulation* clone() const override   { return new Articulation(*this); }
      virtual ElementType type() const override    { return ElementType::ARTICULATION; }

      virtual qreal mag() const override;

      SymId symId() const                       { return _symId; }
      void setSymId(SymId id);
      virtual int subtype() const override      { return int(_symId); }
      QString userName() const;
      const char* articulationName() const;  // type-name of articulation; used for midi rendering
      static const char* symId2ArticulationName(SymId symId);

      virtual void layout() override;

      virtual void read(XmlReader&) override;
      virtual void write(XmlWriter& xml) const override;
      virtual bool readProperties(XmlReader&) override;

      virtual void reset() override;
      virtual QLineF dragAnchor() const override;

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;
      virtual void resetProperty(Pid id) override;
      Sid getPropertyStyle(Pid id) const override;

      bool up() const                       { return _up; }
      void setUp(bool val);
      void setDirection(Direction d)        { _direction = d;    }
      Direction direction() const           { return _direction; }

      ChordRest* chordRest() const;
      Segment* segment() const;
      Measure* measure() const;
      System* system() const;
      Page* page() const;

      ArticulationAnchor anchor() const     { return _anchor;      }
      void setAnchor(ArticulationAnchor v)  { _anchor = v;         }

      MScore::OrnamentStyle ornamentStyle() const { return _ornamentStyle; }
      void setOrnamentStyle(MScore::OrnamentStyle val) { _ornamentStyle = val; }

      bool playArticulation() const { return _playArticulation;}
      void setPlayArticulation(bool val) { _playArticulation = val; }

      QString channelName() const           { return _channelName; }
      void setChannelName(const QString& s) { _channelName = s;    }

      QString accessibleInfo() const override;

      bool isTenuto() const;
      bool isStaccato() const;
      bool isAccent() const;
      bool isMarcato() const;
      bool isLuteFingering() const;

      void doAutoplace();
      };

}     // namespace Ms
#endif

