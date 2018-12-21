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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

/**
 \file
 Definition of classes SysStaff and System
*/

#include "element.h"
#include "spatium.h"
#include "symbol.h"
#include "skyline.h"

namespace Ms {

class Staff;
class StaffLines;
class Clef;
class Page;
class Bracket;
class Lyrics;
class Segment;
class MeasureBase;
class Text;
class InstrumentName;
class SpannerSegment;
class VBox;
class BarLine;

//---------------------------------------------------------
//   SysStaff
///  One staff in a System.
//---------------------------------------------------------

class SysStaff {
      QRectF _bbox;                 // Bbox of StaffLines.
      Skyline _skyline;
      qreal _yOff { 0    };         // offset of top staff line within bbox
      bool _show  { true };         // derived from Staff or false if empty
                                    // staff is hidden
   public:
      int idx     { 0    };
      QList<InstrumentName*> instrumentNames;

      const QRectF& bbox() const    { return _bbox; }
      QRectF& bbox()                { return _bbox; }
      void setbbox(const QRectF& r) { _bbox = r; }
      qreal y() const               { return _bbox.y() + _yOff; }
      void setYOff(qreal offset)    { _yOff = offset; }

      bool show() const             { return _show; }
      void setShow(bool v)          { _show = v; }

      const Skyline& skyline() const { return _skyline; }
      Skyline& skyline()             { return _skyline; }

      SysStaff() {}
      ~SysStaff();
      };

//---------------------------------------------------------
//   System
///    One row of measures for all instruments;
///    a complete piece of the timeline.
//---------------------------------------------------------

class System final : public Element {
      SystemDivider* _systemDividerLeft    { 0 };     // to the next system
      SystemDivider* _systemDividerRight   { 0 };

      std::vector<MeasureBase*> ml;
      QList<SysStaff*> _staves;
      QList<Bracket*> _brackets;
      QList<SpannerSegment*> _spannerSegments;

      qreal _leftMargin              { 0.0    };     ///< left margin for instrument name, brackets etc.
      mutable bool fixedDownDistance { false  };
      qreal _distance;                               // temp. variable used during layout

      SysStaff* firstVisibleSysStaff() const;
      SysStaff* lastVisibleSysStaff() const;

   public:
      System(Score*);
      ~System();
      virtual System* clone() const override      { return new System(*this); }
      virtual ElementType type() const override   { return ElementType::SYSTEM; }

      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void change(Element* o, Element* n) override;
      virtual void write(XmlWriter&) const override;
      virtual void read(XmlReader&) override;

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      void appendMeasure(MeasureBase*);
      void removeMeasure(MeasureBase*);
      void removeLastMeasure();

      Page* page() const                    { return (Page*)parent(); }

      void layoutSystem(qreal);

      void layout2();                     ///< Called after Measure layout.
      void clear();                       ///< Clear measure list.

      QRectF bboxStaff(int staff) const      { return _staves[staff]->bbox(); }
      QList<SysStaff*>* staves()             { return &_staves;   }
      const QList<SysStaff*>* staves() const { return &_staves;   }
      qreal staffYpage(int staffIdx) const;
      qreal staffCanvasYpage(int staffIdx) const;
      SysStaff* staff(int staffIdx) const    { return _staves[staffIdx]; }

      bool pageBreak() const;

      SysStaff* insertStaff(int);
      void removeStaff(int);

      int y2staff(qreal y) const;
      void setInstrumentNames(bool longName, int tick = 0);
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      const std::vector<MeasureBase*>& measures() const { return ml; }

      MeasureBase* measure(int idx)          { return ml[idx]; }
      Measure* firstMeasure() const;
      Measure* lastMeasure() const;
      int endTick() const;

      MeasureBase* prevMeasure(const MeasureBase*) const;
      MeasureBase* nextMeasure(const MeasureBase*) const;

      qreal leftMargin() const    { return _leftMargin; }
      Box* vbox() const;

      const QList<Bracket*>& brackets() const { return _brackets; }

      QList<SpannerSegment*>& spannerSegments()             { return _spannerSegments; }
      const QList<SpannerSegment*>& spannerSegments() const { return _spannerSegments; }

      SystemDivider* systemDividerLeft() const  { return _systemDividerLeft; }
      SystemDivider* systemDividerRight() const { return _systemDividerRight; }

      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;

      qreal minDistance(System*) const;
      qreal topDistance(int staffIdx, const SkylineLine&) const;
      qreal bottomDistance(int staffIdx, const SkylineLine&) const;
      qreal minTop() const;
      qreal minBottom() const;

      void moveBracket(int staffIdx, int srcCol, int dstCol);
      bool hasFixedDownDistance() const { return fixedDownDistance; }
      int firstVisibleStaff() const;
      int nextVisibleStaff(int) const;
      qreal distance() const { return _distance; }
      void setDistance(qreal d) { _distance = d; }
      };

typedef QList<System*>::iterator iSystem;
typedef QList<System*>::const_iterator ciSystem;


}     // namespace Ms
#endif

