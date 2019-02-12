//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BEAM_H__
#define __BEAM_H__

#include "element.h"
#include "durationtype.h"
#include "property.h"

namespace Ms {

class ChordRest;
class MuseScoreView;
class Chord;
class System;
class Skyline;

enum class SpannerSegmentType;

struct BeamFragment;

//---------------------------------------------------------
//   @@ Beam
//---------------------------------------------------------

class Beam final : public Element {
      Q_GADGET
      QVector<ChordRest*> _elements;        // must be sorted by tick
      QVector<QLineF*> beamSegments;
      Direction _direction;

      bool _up;
      bool _distribute;                   // equal spacing of elements
      bool _noSlope;

      bool _userModified[2];              // 0: auto/down  1: up
      bool _isGrace;
      bool _cross;

      qreal _grow1;                       // define "feather" beams
      qreal _grow2;
      qreal _beamDist;

      QVector<BeamFragment*> fragments;     // beam splits across systems

      mutable int _id;          // used in read()/write()

      int minMove;              // set in layout1()
      int maxMove;
      TDuration maxDuration;
      qreal slope { 0.0 };

      void layout2(std::vector<ChordRest*>, SpannerSegmentType, int frag);
      bool twoBeamedNotes();
      void computeStemLen(const std::vector<ChordRest*>& crl, qreal& py1, int beamLevels);
      bool slopeZero(const std::vector<ChordRest*>& crl);
      bool hasNoSlope();
      void addChordRest(ChordRest* a);
      void removeChordRest(ChordRest* a);

   public:
      enum class Mode : signed char {
            AUTO, BEGIN, MID, END, NONE, BEGIN32, BEGIN64, INVALID = -1
            };
      Q_ENUM(Mode)

      Beam(Score* = 0);
      Beam(const Beam&);
      ~Beam();
      virtual Beam* clone() const override         { return new Beam(*this); }
      virtual ElementType type() const override    { return ElementType::BEAM; }
      virtual QPointF pagePos() const override;    ///< position in page coordinates
      virtual QPointF canvasPos() const override;  ///< position in page coordinates

      virtual bool isEditable() const override { return true; }
      virtual void startEdit(EditData&) override;
      virtual void endEdit(EditData&) override;
      virtual void editDrag(EditData&) override;
      virtual void updateGrips(EditData&) const override;

      virtual int tick() const override;
      virtual int rtick() const override;

      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual void reset() override;

      System* system() const { return toSystem(parent()); }

      void layout1();
      void layoutGraceNotes();
      void layout();

      const QVector<ChordRest*>& elements() { return _elements;  }
      void clear()                        { _elements.clear(); }
      bool empty() const                { return _elements.empty(); }

      virtual void add(Element*) override;
      virtual void remove(Element*) override;

      virtual void move(const QPointF&) override;
      virtual void draw(QPainter*) const override;

      bool up() const                     { return _up; }
      void setUp(bool v)                  { _up = v;    }
      void setId(int i) const             { _id = i;    }
      int id() const                      { return _id; }
      bool noSlope() const                { return _noSlope; }
      void setNoSlope(bool val)           { _noSlope = val; }

      void setBeamDirection(Direction d);
      Direction beamDirection() const     { return _direction; }

      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&) override;

      qreal growLeft() const              { return _grow1; }
      qreal growRight() const             { return _grow2; }
      void setGrowLeft(qreal val)         { _grow1 = val;  }
      void setGrowRight(qreal val)        { _grow2 = val;  }

      bool distribute() const             { return _distribute; }
      void setDistribute(bool val)        { _distribute = val;  }

      bool userModified() const;
      void setUserModified(bool val);

      QPointF beamPos() const;
      void setBeamPos(const QPointF& bp);

      qreal beamDist() const              { return _beamDist; }

      virtual QVariant getProperty(Pid propertyId) const override;
      virtual bool setProperty(Pid propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(Pid id) const override;

      bool isGrace() const { return _isGrace; }  // for debugger
      bool cross() const   { return _cross; }

      void addSkyline(Skyline&);

      virtual void triggerLayout() const override;
      };


}     // namespace Ms
#endif
