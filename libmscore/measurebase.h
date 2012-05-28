//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: measurebase.h 5632 2012-05-15 16:36:57Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __MEASUREBASE_H__
#define __MEASUREBASE_H__

/**
 \file
 Definition of MeasureBase class.
*/

#include "element.h"

class Score;
class System;
class Measure;
class LayoutBreak;

//---------------------------------------------------------
//   MeasureWidth
//---------------------------------------------------------

/**
 result of layoutX().
*/

struct MeasureWidth {
      qreal stretchable;
      qreal nonStretchable;

      MeasureWidth() {}
      MeasureWidth(qreal a, qreal b) {
            stretchable = a;
            nonStretchable = b;
            }
      };

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

/**
      Base class for Measure, HBox and VBox
*/

class MeasureBase : public Element {
      Q_OBJECT

      MeasureBase* _next;
      MeasureBase* _prev;
      int _tick;

   protected:
      MeasureWidth _mw;
      ElementList _el;        ///< Measure(/tick) relative -elements: with defined start time
                              ///< but outside the staff

      bool _dirty;
      bool _lineBreak;        ///< Forced line break
      bool _pageBreak;        ///< Forced page break
      LayoutBreak* _sectionBreak;

   public:
      MeasureBase(Score* score);
      ~MeasureBase();
      MeasureBase(const MeasureBase&);
      virtual MeasureBase* clone() const = 0;
      virtual void setScore(Score* s);

      MeasureBase* next() const              { return _next;   }
      void setNext(MeasureBase* e)           { _next = e;      }
      MeasureBase* prev() const              { return _prev;   }
      void setPrev(MeasureBase* e)           { _prev = e;      }

      Measure* nextMeasure() const;
      Measure* prevMeasure() const;

      virtual int ticks() const              { return 0;       }
      virtual void write(Xml&, int, bool) const = 0;

      void layout0();
      virtual void layout();

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      MeasureWidth& layoutWidth()            { return _mw;        }
      void setLayoutWidth(const MeasureWidth& w) { _mw = w; }
      ElementList* el()                      { return &_el; }
      const ElementList* el() const          { return &_el; }
      System* system() const                 { return (System*)parent(); }
      void setSystem(System* s)              { setParent((Element*)s);   }

      bool lineBreak() const                 { return _lineBreak; }
      bool pageBreak() const                 { return _pageBreak; }
      LayoutBreak* sectionBreak() const      { return _sectionBreak; }
      void setLineBreak(bool v)              { _lineBreak = v;    }
      void setPageBreak(bool v)              { _pageBreak = v;    }
      void setSectionBreak(LayoutBreak* v)   { _sectionBreak = v; }

      virtual void moveTicks(int diff)       { setTick(tick() + diff); }

      virtual qreal distanceUp(int) const       { return 0.0; }
      virtual qreal distanceDown(int) const     { return 0.0; }
      virtual qreal userDistanceUp(int) const   { return .0;  }
      virtual qreal userDistanceDown(int) const { return .0;  }

      virtual void add(Element*);
      virtual void remove(Element*);
      void setDirty(bool val = true)         { _dirty = val;  }
      bool dirty() const                     { return _dirty; }
      int tick() const                       { return _tick;  }
      int endTick() const                    { return tick() + ticks();  }
      void setTick(int t)                    { _tick = t;     }

      qreal pause() const;
      };

#endif

