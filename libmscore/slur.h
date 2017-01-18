//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SLUR_H__
#define __SLUR_H__

#include "slurtie.h"

namespace Ms {

//---------------------------------------------------------
//   @@ SlurSegment
///    a single segment of slur; also used for Tie
//---------------------------------------------------------

class SlurSegment : public SlurTieSegment {
      Q_OBJECT

   protected:
      void changeAnchor(MuseScoreView*, Grip, Element*);

   public:
      SlurSegment(Score* s) : SlurTieSegment(s) {}
      SlurSegment(const SlurSegment& ss) : SlurTieSegment(ss) {}

      virtual SlurSegment* clone() const override  { return new SlurSegment(*this); }
      virtual ElementType type() const override  { return ElementType::SLUR_SEGMENT; }
      virtual int subtype() const override         { return static_cast<int>(spanner()->type()); }
      virtual QString subtypeName() const override { return name(spanner()->type()); }
      virtual void draw(QPainter*) const override;

      void layoutSegment(const QPointF& p1, const QPointF& p2);

      bool isEdited() const;
      virtual void editDrag(const EditData&) override;
      virtual bool edit(MuseScoreView*, Grip grip, int key, Qt::KeyboardModifiers, const QString& s) override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return int(Grip::GRIPS); }
      virtual QPointF gripAnchor(Grip grip) const override;

      QPointF getGrip(Grip) const override;
      void setGrip(Grip, const QPointF&) override;

      Slur* slur() const { return (Slur*)spanner(); }

      void computeBezier(QPointF so = QPointF());
      };

//---------------------------------------------------------
//   @@ Slur
//---------------------------------------------------------

class Slur : public SlurTie {
      Q_OBJECT

      void slurPosChord(SlurPos*);

   public:
      Slur(Score* = 0);
      ~Slur();
      virtual Slur* clone() const override        { return new Slur(*this); }
      virtual ElementType type() const override { return ElementType::SLUR; }
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void layout() override;
      virtual SpannerSegment* layoutSystem(System*) override;
      virtual void setTrack(int val) override;
      virtual void slurPos(SlurPos*) override;

      bool readProperties(XmlReader&);

      SlurSegment* frontSegment() const   { return (SlurSegment*)spannerSegments().front(); }
      SlurSegment* backSegment() const    { return (SlurSegment*)spannerSegments().back();  }
      SlurSegment* takeLastSegment()      { return (SlurSegment*)spannerSegments().takeLast(); }
      SlurSegment* segmentAt(int n) const { return (SlurSegment*)spannerSegments().at(n); }
      virtual SlurTieSegment* newSlurTieSegment() override { return new SlurSegment(score()); }
      };

}     // namespace Ms
#endif

