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

class QPainter;

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
      TOP_CHORD,      // attribute is alway placed at top of chord
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

class Articulation : public Element {
      Q_OBJECT

      SymId _symId;
      Direction _direction;
      QString _channelName;

      ArticulationAnchor _anchor;

      bool _up;
      qreal _timeStretch;                       // for fermata
      MScore::OrnamentStyle _ornamentStyle;     // for use in ornaments such as trill
      bool _playArticulation;

      virtual void draw(QPainter*) const;

   public:
      Articulation(Score*);
      Articulation(SymId, Score*);
      Articulation &operator=(const Articulation&) = delete;

      virtual Articulation* clone() const override   { return new Articulation(*this); }
      virtual Element::Type type() const override    { return Element::Type::ARTICULATION; }

      virtual qreal mag() const override;


      SymId symId() const                       { return _symId; }
      void setSymId(SymId id);
      virtual int subtype() const override      { return int(_symId); }
      QString userName() const;
      const char* articulationName() const;  // type-name of articulation; used for midi rendering

      virtual void layout() override;

      virtual void read(XmlReader&) override;
      virtual void write(Xml& xml) const override;
      virtual bool readProperties(XmlReader&) override;

      virtual void reset() override;
      virtual QLineF dragAnchor() const override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      StyleIdx getPropertyStyle(P_ID id) const override;

      bool up() const                       { return _up; }
      void setUp(bool val);
      void setDirection(Direction d);
      Direction direction() const           { return _direction; }

      ChordRest* chordRest() const;
      Segment* segment() const;
      Measure* measure() const;
      System* system() const;
      Page* page() const;

      ArticulationAnchor anchor() const     { return _anchor;      }
      void setAnchor(ArticulationAnchor v)  { _anchor = v;         }

      qreal timeStretch() const             { return _timeStretch; }
      void setTimeStretch(qreal val)        { _timeStretch = val;  }

      MScore::OrnamentStyle ornamentStyle() const { return _ornamentStyle; }
      void setOrnamentStyle(MScore::OrnamentStyle val) { _ornamentStyle = val; }

      bool playArticulation() const { return _playArticulation;}
      void setPlayArticulation(bool val) { _playArticulation = val; }

      QString channelName() const           { return _channelName; }
      void setChannelName(const QString& s) { _channelName = s;    }

      QString accessibleInfo() const override;

      bool isFermata() const;
      bool isTenuto() const;
      bool isStaccato() const;
      bool isAccent() const;
      bool isLuteFingering() const;
      };

}     // namespace Ms
#endif

