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

class ChordRest;
class MuseScoreView;
class Chord;
class QPainter;

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
      qreal _grow1;                       // define "feather" beams
      qreal _grow2;
      qreal _beamDist;

      QList<BeamFragment*> fragments;     // beam splits across systems

      bool _userModified[2];    // 0: auto/down  1: up

      mutable int _id;          // used in read()/write()

      int minMove;              // set in layout1()
      int maxMove;
      bool isGrace;
      bool cross;
      TDuration maxDuration;
      qreal slope;

      int editFragment;       // valid in edit mode

      void layout2(QList<ChordRest*>, SpannerSegmentType, int frag);
      bool twoBeamedNotes();
      void computeStemLen(const QList<ChordRest*>& crl, qreal& py1, int beamLevels);
      bool noSlope(const QList<ChordRest*>& crl);

   public:
      Beam(Score* s);
      Beam(const Beam&);
      ~Beam();
      virtual Beam* clone() const         { return new Beam(*this); }
      virtual ElementType type() const    { return BEAM; }
      virtual QPointF pagePos() const;  ///< position in page coordinates

      virtual bool isEditable() const { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;

      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

      virtual void reset();

      System* system() const { return (System*)parent(); }

      void layout1();
      void layout();

      const QList<ChordRest*>& elements() { return _elements;  }
      void clear()                        { _elements.clear(); }
      bool isEmpty() const                { return _elements.isEmpty(); }
      virtual void add(ChordRest* a);
      virtual void remove(ChordRest* a);
      virtual void move(qreal, qreal);
      virtual void draw(QPainter*) const;
      bool up() const                     { return _up; }
      void setUp(bool v)                  { _up = v;    }
      void setId(int i) const             { _id = i;    }
      int id() const                      { return _id; }

      void setBeamDirection(MScore::Direction d);
      MScore::Direction beamDirection() const     { return _direction; }

      virtual QPainterPath shape() const;
      virtual bool contains(const QPointF& p) const;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);

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

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };

extern bool endBeam(const Fraction&, ChordRest* cr, ChordRest* prevCr);
#endif

