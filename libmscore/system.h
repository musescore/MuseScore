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
///  One staff of a System.
//---------------------------------------------------------

class SysStaff {
      QRectF _bbox;           ///< Bbox of StaffLines.
      qreal _distanceUp;      ///< distance to previous staff
      qreal _distanceDown;    ///< distance to next staff
      bool _show;             ///< derived from Staff or false if empty
                              ///< staff is hidden
   public:
      int idx;
      QList<InstrumentName*> instrumentNames;

      const QRectF& bbox() const    { return _bbox; }
      QRectF& bbox()                { return _bbox; }
      QRectF& rbb()                 { return _bbox; }
      qreal y() const               { return _bbox.y(); }
      qreal right() const           { return _bbox.right(); }
      void setbbox(const QRectF& r) { _bbox = r; }

      qreal distanceUp() const      { return _distanceUp;   }
      void setDistanceUp(qreal v)   { _distanceUp = v;      }
      qreal distanceDown() const    { return _distanceDown; }
      void setDistanceDown(qreal v) { _distanceDown = v;    }

      bool show() const             { return _show; }
      void setShow(bool v)          { _show = v; }

      SysStaff();
      ~SysStaff();
      };

//---------------------------------------------------------
//   @@ System
///  One row of measures for all instruments;
///  a complete piece of the timeline.
//---------------------------------------------------------

class System : public Element {
      Q_OBJECT

      QList<MeasureBase*> ml;
      QList<SysStaff*> _staves;
      QList<Bracket*> _brackets;

      BarLine* _barLine;      ///< Left hand bar, connects staves in system.
      qreal _leftMargin;      ///< left margin for instrument name, brackets etc.
      bool _pageBreak;
      bool _firstSystem;      ///< used to decide between long and short instrument
                              ///< names; set by score()->doLayout()
      bool _vbox;             ///< contains only one VBox in ml
      bool _sameLine;
      bool _addStretch;

      QList<SpannerSegment*> _spannerSegments;

      qreal _stretchDistance;
      qreal _distance;

      void setDistanceUp(int n, qreal v)   { _staves[n]->setDistanceUp(v); }
      void setDistanceDown(int n, qreal v) { _staves[n]->setDistanceDown(v); }

   public:
      System(Score*);
      ~System();
      virtual System* clone() const    { return new System(*this); }
      virtual ElementType type() const { return SYSTEM; }

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      Page* page() const                 { return (Page*)parent(); }

      virtual void layout(qreal xoffset);
      void layout2();                     ///< Called after Measure layout.
      void clear();                       ///< Clear measure list.

      QRectF bboxStaff(int staff) const      { return _staves[staff]->bbox(); }
      QList<SysStaff*>* staves()             { return &_staves;   }
      const QList<SysStaff*>* staves() const { return &_staves;   }
      qreal staffYpage(int staffIdx) const;
#ifdef NDEBUG
      SysStaff* staff(int staffIdx) const    { return _staves[staffIdx]; }
#else
      SysStaff* staff(int staffIdx) const    {
            if (staffIdx >= _staves.size()) {
                  qDebug("System::staff(): bad index %d", staffIdx);
                  staffIdx = _staves.size() - 1;
                  // abort();
                  }
            return _staves[staffIdx];
            }
#endif

      qreal distanceUp(int idx) const        { return _staves[idx]->distanceUp(); }
      qreal distanceDown(int idx) const      { return _staves[idx]->distanceDown(); }
      bool pageBreak() const                 { return _pageBreak; }
      void setPageBreak(bool val)            { _pageBreak = val; }

      SysStaff* insertStaff(int);
      void removeStaff(int);

      BarLine* barLine() const               { return _barLine; }
      int y2staff(qreal y) const;
      void setInstrumentNames(bool longName);
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      QList<MeasureBase*>& measures()        { return ml; }
      MeasureBase* measure(int idx)          { return ml[idx]; }
      Measure* firstMeasure() const;
      Measure* lastMeasure() const;

      MeasureBase* prevMeasure(const MeasureBase*) const;
      MeasureBase* nextMeasure(const MeasureBase*) const;

      qreal leftMargin() const    { return _leftMargin; }
      void setFirstSystem(bool v) { _firstSystem = v;   }
      bool isVbox() const         { return _vbox;       }
      VBox* vbox() const          { return (VBox*)ml[0];       }
      void setVbox(bool v)        { _vbox = v;          }

      void layoutLyrics(Lyrics*, Segment*, int staffIdx);

      bool addStretch() const     { return _addStretch; }
      void setAddStretch(bool v)  { _addStretch = v; }
      bool sameLine() const       { return _sameLine;   }
      void setSameLine(bool v)    { _sameLine = v; }

      qreal stretchDistance() const      { return _stretchDistance; }
      void setStretchDistance(qreal val) { _stretchDistance = val;  }
      void addStretchDistance(qreal val) { _stretchDistance += val;  }
      qreal distance() const             { return _distance; }
      void setDistance(qreal val)        { _distance = val;  }
      QList<Bracket*>& brackets()        { return _brackets; }

      QList<SpannerSegment*>& spannerSegments()             { return _spannerSegments; }
      const QList<SpannerSegment*>& spannerSegments() const { return _spannerSegments; }
      };

typedef QList<System*>::iterator iSystem;
typedef QList<System*>::const_iterator ciSystem;


}     // namespace Ms
#endif

