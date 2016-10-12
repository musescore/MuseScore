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

#if 1
//---------------------------------------------------------
//   Repeat
//---------------------------------------------------------

enum class Repeat : char {
      NONE    = 0,
      END     = 1,
      START   = 2,
      MEASURE = 4,
      JUMP    = 8
      };

constexpr Repeat operator| (Repeat t1, Repeat t2) {
      return static_cast<Repeat>(static_cast<int>(t1) | static_cast<int>(t2));
      }
constexpr bool operator& (Repeat t1, Repeat t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }
#endif

//---------------------------------------------------------
//   @@ MeasureBase
///    Virtual base class for Measure, HBox and VBox
//
//   @P lineBreak       bool        true if a system break is positioned on this measure
//   @P nextMeasure     Measure     the next Measure (read-only)
//   @P nextMeasureMM   Measure     the next multi-measure rest Measure (read-only)
//   @P pageBreak       bool        true if a page break is positioned on this measure
//   @P prevMeasure     Measure     the previous Measure (read-only)
//   @P prevMeasureMM   Measure     the previous multi-measure rest Measure (read-only)
//---------------------------------------------------------

class MeasureBase : public Element {
      Q_OBJECT

      Q_PROPERTY(bool         lineBreak         READ lineBreak   WRITE undoSetLineBreak)
      Q_PROPERTY(Ms::Measure* nextMeasure       READ nextMeasure)
      Q_PROPERTY(Ms::Measure* nextMeasureMM     READ nextMeasureMM)
      Q_PROPERTY(bool         pageBreak         READ pageBreak   WRITE undoSetPageBreak)
      Q_PROPERTY(Ms::Measure* prevMeasure       READ prevMeasure)
      Q_PROPERTY(Ms::Measure* prevMeasureMM     READ prevMeasureMM)

      MeasureBase* _next    { 0 };
      MeasureBase* _prev    { 0 };

      ElementList _el;                    ///< Measure(/tick) relative -elements: with defined start time
                                          ///< but outside the staff
      LayoutBreak* _sectionBreak { 0 };

      int _tick              { 0 };
      int _no                { 0 };       ///< Measure number, counting from zero
      int _noOffset          { 0 };       ///< Offset to measure number

      bool _repeatEnd        { false };
      bool _repeatStart      { false };
      bool _repeatJump       { false };
      bool _irregular        { true  };        ///< Irregular measure, do not count

      bool _lineBreak        { false };        ///< Forced line break
      bool _pageBreak        { false };        ///< Forced page break
      bool _noBreak          { false };

      bool _hasSystemHeader  { false };
      bool _hasSystemTrailer { false };
      bool _hasCourtesyKeySig { false };

   protected:

   public:
      MeasureBase(Score* score = 0);
      ~MeasureBase();
      MeasureBase(const MeasureBase&);

      virtual MeasureBase* clone() const = 0;
      virtual Element::Type type() const = 0;

      virtual void setScore(Score* s) override;

      MeasureBase* next() const              { return _next;   }
      MeasureBase* nextMM() const;
      void setNext(MeasureBase* e)           { _next = e;      }
      MeasureBase* prev() const              { return _prev;   }
      void setPrev(MeasureBase* e)           { _prev = e;      }

      Ms::Measure* nextMeasure() const;
      Ms::Measure* prevMeasure() const;
      Ms::Measure* nextMeasureMM() const;
      Ms::Measure* prevMeasureMM() const;

      virtual void write(Xml&) const override = 0;
      virtual void write(Xml&, int, bool) const = 0;

      virtual void layout();

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      ElementList& el()                      { return _el; }
      const ElementList& el() const          { return _el; }
      System* system() const                 { return (System*)parent(); }
      void setSystem(System* s)              { setParent((Element*)s);   }

      bool lineBreak() const                 { return _lineBreak; }
      bool pageBreak() const                 { return _pageBreak; }
      bool noBreak() const                   { return _noBreak;   }
      LayoutBreak* sectionBreak() const      { return _sectionBreak; }
      void setLineBreak(bool v)              { _lineBreak = v;    }
      void setPageBreak(bool v)              { _pageBreak = v;    }
      void setSectionBreak(LayoutBreak* v)   { _sectionBreak = v; }
      void setNoBreak(bool v)                { _noBreak = v;      }

      void undoSetBreak(bool v, LayoutBreak::Type type);
      void undoSetLineBreak(bool v)          {  undoSetBreak(v, LayoutBreak::LINE);}
      void undoSetPageBreak(bool v)          {  undoSetBreak(v, LayoutBreak::PAGE);}
      void undoSetSectionBreak(bool v)       {  undoSetBreak(v, LayoutBreak::SECTION);}
      void undoSetNoBreak(bool v)            {  undoSetBreak(v, LayoutBreak::NOBREAK);}

      virtual void moveTicks(int diff)       { setTick(tick() + diff); }

      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void writeProperties(Xml&) const override;
      virtual bool readProperties(XmlReader&) override;

      virtual int tick() const override      { return _tick;  }
      virtual int ticks() const              { return 0;      }
      int endTick() const                    { return tick() + ticks();  }
      void setTick(int t)                    { _tick = t;     }

      qreal pause() const;

      virtual QVariant getProperty(P_ID) const override;
      virtual bool setProperty(P_ID, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      void clearElements();
      ElementList takeElements();

      int no() const                     { return _no;          }
      void setNo(int n)                  { _no = n;             }
      bool irregular() const             { return _irregular;   }
      void setIrregular(bool val)        { _irregular = val;    }
      int noOffset() const               { return _noOffset;    }
      void setNoOffset(int n)            { _noOffset = n;       }

      bool repeatEnd() const             { return _repeatEnd;     }
      bool repeatStart() const           { return _repeatStart;   }
      bool repeatJump() const            { return _repeatJump;    }

      void setRepeatEnd(bool v)          { _repeatEnd = v;     }
      void setRepeatStart(bool v)        { _repeatStart = v;   }
      void setRepeatJump(bool v)         { _repeatJump = v;    }


      bool hasSystemHeader() const       { return _hasSystemHeader;    }
      bool hasSystemTrailer() const      { return _hasSystemTrailer;   }
      void setHasSystemHeader(bool val)  { _hasSystemHeader = val;     }
      void setHasSystemTrailer(bool val) { _hasSystemTrailer = val;    }

      bool hasCourtesyKeySig() const     { return _hasCourtesyKeySig; }
      void setHasCourtesyKeySig(int val) { _hasCourtesyKeySig = val; }

      int index() const;
      };


}     // namespace Ms
#endif

