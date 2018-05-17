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
      MeasureBase* _next    { 0 };
      MeasureBase* _prev    { 0 };

      ElementList _el;                    ///< Measure(/tick) relative -elements: with defined start time
                                          ///< but outside the staff
      int _tick              { 0 };
      int _no                { 0 };       ///< Measure number, counting from zero
      int _noOffset          { 0 };       ///< Offset to measure number

   protected:
      void cleanupLayoutBreaks(bool undo);

   public:
      MeasureBase(Score* score = 0);
      ~MeasureBase();
      MeasureBase(const MeasureBase&);

      virtual MeasureBase* clone() const = 0;
      virtual ElementType type() const = 0;

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

      virtual void write(XmlWriter&) const override = 0;
      virtual void write(XmlWriter&, int, bool, bool) const = 0;

      virtual void layout();

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      ElementList& el()                      { return _el; }
      const ElementList& el() const          { return _el; }
      System* system() const                 { return (System*)parent(); }
      void setSystem(System* s)              { setParent((Element*)s);   }

      LayoutBreak* sectionBreakElement() const;

      void undoSetBreak(bool v, LayoutBreak::Type type);
      void undoSetLineBreak(bool v)          {  undoSetBreak(v, LayoutBreak::LINE);}
      void undoSetPageBreak(bool v)          {  undoSetBreak(v, LayoutBreak::PAGE);}
      void undoSetSectionBreak(bool v)       {  undoSetBreak(v, LayoutBreak::SECTION);}
      void undoSetNoBreak(bool v)            {  undoSetBreak(v, LayoutBreak::NOBREAK);}

      virtual void moveTicks(int diff)       { setTick(tick() + diff); }

      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void writeProperties(XmlWriter&) const override;
      virtual bool readProperties(XmlReader&) override;
      virtual bool readProperties300(XmlReader&) override;

      virtual int tick() const override      { return _tick;  }
      virtual int ticks() const              { return 0;      }
      int endTick() const                    { return tick() + ticks();  }
      void setTick(int t)                    { _tick = t;     }

      qreal pause() const;

      virtual QVariant getProperty(Pid) const override;
      virtual bool setProperty(Pid, const QVariant&) override;
      virtual QVariant propertyDefault(Pid) const override;

      void clearElements();
      ElementList takeElements();

      int no() const                   { return _no;                     }
      void setNo(int n)                { _no = n;                        }
      int noOffset() const             { return _noOffset;               }
      void setNoOffset(int n)          { _noOffset = n;                  }

      bool repeatEnd() const           { return flag(ElementFlag::REPEAT_END);    }
      void setRepeatEnd(bool v)        { setFlag(ElementFlag::REPEAT_END, v);     }

      bool repeatStart() const         { return flag(ElementFlag::REPEAT_START);  }
      void setRepeatStart(bool v)      { setFlag(ElementFlag::REPEAT_START, v);   }

      bool repeatJump() const          { return flag(ElementFlag::REPEAT_JUMP);   }
      void setRepeatJump(bool v)       { setFlag(ElementFlag::REPEAT_JUMP, v);    }

      bool irregular() const           { return flag(ElementFlag::IRREGULAR);     }
      void setIrregular(bool v)        { setFlag(ElementFlag::IRREGULAR, v);      }

      bool lineBreak() const           { return flag(ElementFlag::LINE_BREAK);    }
      void setLineBreak(bool v)        { setFlag(ElementFlag::LINE_BREAK, v);     }

      bool pageBreak() const           { return flag(ElementFlag::PAGE_BREAK);    }
      void setPageBreak(bool v)        { setFlag(ElementFlag::PAGE_BREAK, v);     }

      bool sectionBreak() const        { return flag(ElementFlag::SECTION_BREAK); }
      void setSectionBreak(bool v)     { setFlag(ElementFlag::SECTION_BREAK, v);  }

      bool noBreak() const             { return flag(ElementFlag::NO_BREAK);      }
      void setNoBreak(bool v)          { setFlag(ElementFlag::NO_BREAK, v);       }

      bool hasCourtesyKeySig() const   { return flag(ElementFlag::KEYSIG);        }
      void setHasCourtesyKeySig(int v) { setFlag(ElementFlag::KEYSIG, v);         }

      virtual void computeMinWidth() { };

      int index() const;
      };


}     // namespace Ms
#endif

