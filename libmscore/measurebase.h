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
//   @@ MeasureBase
///   Virtual base class for Measure, HBox and VBox
//---------------------------------------------------------

class MeasureBase : public Element {
      Q_OBJECT

      MeasureBase* _next;
      MeasureBase* _prev;
      int _tick;

   protected:
      ElementList _el;        ///< Measure(/tick) relative -elements: with defined start time
                              ///< but outside the staff

      bool _breakHint;
      bool _lineBreak;        ///< Forced line break
      bool _pageBreak;        ///< Forced page break
      LayoutBreak* _sectionBreak;

   public:
      MeasureBase(Score* score = 0);
      ~MeasureBase();
      MeasureBase(const MeasureBase&);
      virtual MeasureBase* clone() const = 0;
      virtual void setScore(Score* s);

      MeasureBase* next() const              { return _next;   }
      void setNext(MeasureBase* e)           { _next = e;      }
      MeasureBase* prev() const              { return _prev;   }
      void setPrev(MeasureBase* e)           { _prev = e;      }

      Q_INVOKABLE Measure* nextMeasure() const;
      Q_INVOKABLE Measure* prevMeasure() const;

      virtual int ticks() const              { return 0;       }
      virtual void write(Xml&, int, bool) const = 0;

      void layout0();
      virtual void layout();

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      ElementList* el()                      { return &_el; }
      const ElementList* el() const          { return &_el; }
      System* system() const                 { return (System*)parent(); }
      void setSystem(System* s)              { setParent((Element*)s);   }

      bool breakHint() const                 { return _breakHint;   }
      void setBreakHint(bool val)            { _breakHint = val;  }

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
      int tick() const                       { return _tick;  }
      int endTick() const                    { return tick() + ticks();  }
      void setTick(int t)                    { _tick = t;     }

      qreal pause() const;

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      };

#endif

