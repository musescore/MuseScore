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

#ifndef __MEASUREBASE_H__
#define __MEASUREBASE_H__

/**
 \file
 Definition of MeasureBase class.
*/

#include "element.h"
#include "layoutbreak.h"

namespace Ms {

class Score;
class System;
class Measure;

//---------------------------------------------------------
//   @@ MeasureBase
///    Virtual base class for Measure, HBox and VBox
//
//   @P lineBreak       bool              true if a system break is positioned on this measure
//   @P nextMeasure     Ms::Measure       the next Measure (read-only)
//   @P nextMeasureMM   Ms::Measure       the next multi-measure rest Measure (read-only)
//   @P pageBreak       bool              true if a page break is positioned on this measure
//   @P prevMeasure     Ms::Measure       the previous Measure (read-only)
//   @P prevMeasureMM   Ms::Measure       the previous multi-measure rest Measure (read-only)
//---------------------------------------------------------

class MeasureBase : public Element {
      Q_OBJECT

      Q_PROPERTY(bool         lineBreak         READ lineBreak   WRITE undoSetLineBreak)
      Q_PROPERTY(Ms::Measure* nextMeasure       READ nextMeasure)
      Q_PROPERTY(Ms::Measure* nextMeasureMM     READ nextMeasureMM)
      Q_PROPERTY(bool         pageBreak         READ pageBreak   WRITE undoSetPageBreak)
      Q_PROPERTY(Ms::Measure* prevMeasure       READ prevMeasure)
      Q_PROPERTY(Ms::Measure* prevMeasureMM     READ prevMeasureMM)

      MeasureBase* _next;
      MeasureBase* _prev;
      int _tick;
      bool _breakHint;

   protected:
      ElementList _el;        ///< Measure(/tick) relative -elements: with defined start time
                              ///< but outside the staff

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
      MeasureBase* nextMM() const;
      void setNext(MeasureBase* e)           { _next = e;      }
      MeasureBase* prev() const              { return _prev;   }
      void setPrev(MeasureBase* e)           { _prev = e;      }

      Ms::Measure* nextMeasure() const;
      Ms::Measure* prevMeasure() const;
      Ms::Measure* nextMeasureMM() const;
      Ms::Measure* prevMeasureMM() const;

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

      bool lineBreak() const                 { return _lineBreak; }
      bool pageBreak() const                 { return _pageBreak; }
      LayoutBreak* sectionBreak() const      { return _sectionBreak; }
      void setLineBreak(bool v)              { _lineBreak = v;    }
      void setPageBreak(bool v)              { _pageBreak = v;    }
      void setSectionBreak(LayoutBreak* v)   { _sectionBreak = v; }
      void undoSetBreak(bool v, LayoutBreak::Type type);
      void undoSetLineBreak(bool v)          {  undoSetBreak(v, LayoutBreak::Type::LINE);}
      void undoSetPageBreak(bool v)          {  undoSetBreak(v, LayoutBreak::Type::PAGE);}
      void undoSetSectionBreak(bool v)       {  undoSetBreak(v, LayoutBreak::Type::SECTION);}

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


}     // namespace Ms
#endif

