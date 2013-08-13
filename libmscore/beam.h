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
#include "spanner.h"

class QPainter;

namespace Ms {

class ChordRest;
class MuseScoreView;
class Chord;

struct BeamFragment;

//---------------------------------------------------------
//   @@ Beam
//---------------------------------------------------------

class Beam : public Element {
      Q_OBJECT

      QList<ChordRest*> _elements;        // must be sorted by tick
      QList<QLineF*> beamSegments;
      MScore::Direction _direction;

      bool _up;
      bool _distribute;                   // equal spacing of elements
      bool _noSlope;
      PropertyStyle noSlopeStyle;

      bool _userModified[2];              // 0: auto/down  1: up
      bool isGrace;
      bool cross;

      qreal _grow1;                       // define "feather" beams
      qreal _grow2;
      qreal _beamDist;

      QList<BeamFragment*> fragments;     // beam splits across systems

      mutable int _id;          // used in read()/write()

      int minMove;              // set in layout1()
      int maxMove;
      TDuration maxDuration;
      qreal slope;

      int editFragment;       // valid in edit mode

      void layout2(QList<ChordRest*>, SpannerSegmentType, int frag);
      bool twoBeamedNotes();
      void computeStemLen(const QList<ChordRest*>& crl, qreal& py1, int beamLevels);
      bool slopeZero(const QList<ChordRest*>& crl);
      bool hasNoSlope();

   public:
      Beam(Score* s);
      Beam(const Beam&);
      ~Beam();
      virtual Beam* clone() const override         { return new Beam(*this); }
      virtual ElementType type() const override    { return BEAM; }
      virtual QPointF pagePos() const override;    ///< position in page coordinates
      virtual QPointF canvasPos() const override;  ///< position in page coordinates

      virtual bool isEditable() const override { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void editDrag(const EditData&) override;
      virtual void updateGrips(int*, QRectF*) const override;

      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/) override;

      virtual void reset() override;

      System* system() const { return (System*)parent(); }

      void layout1();
      void layoutGraceNotes();
      void layout();

      const QList<ChordRest*>& elements() { return _elements;  }
      void clear()                        { _elements.clear(); }
      bool isEmpty() const                { return _elements.isEmpty(); }

      void add(ChordRest* a);
      void remove(ChordRest* a);

      virtual void move(qreal, qreal) override;
      virtual void draw(QPainter*) const override;

      bool up() const                     { return _up; }
      void setUp(bool v)                  { _up = v;    }
      void setId(int i) const             { _id = i;    }
      int id() const                      { return _id; }
      bool noSlope() const                { return _noSlope; }
      void setNoSlope(bool val)           { _noSlope = val; }

      void setBeamDirection(MScore::Direction d);
      MScore::Direction beamDirection() const     { return _direction; }

      virtual QPainterPath shape() const override;
      virtual bool contains(const QPointF& p) const override;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const override;
      virtual Element* drop(const DropData&) override;

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

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual PropertyStyle propertyStyle(P_ID) const override;
      virtual void resetProperty(P_ID id) override;
      virtual void styleChanged() override;
      };


}     // namespace Ms
#endif

