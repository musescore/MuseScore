//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SPANNER_H__
#define __SPANNER_H__

#include "element.h"

namespace Ms {

class Spanner;

//---------------------------------------------------------
//   SpannerSegmentType
//---------------------------------------------------------

enum class SpannerSegmentType {
      SINGLE, BEGIN, MIDDLE, END
      };

//---------------------------------------------------------
//   @@ SpannerSegment
//!    parent: System
//---------------------------------------------------------

class SpannerSegment : public Element {
      Spanner* _spanner;
      SpannerSegmentType _spannerSegmentType;

   protected:
      QPointF _p2;
      QPointF _offset2;

   public:
      SpannerSegment(Spanner*, Score*, ElementFlags f = ElementFlag::ON_STAFF | ElementFlag::MOVABLE);
      SpannerSegment(Score* s, ElementFlags f = ElementFlag::ON_STAFF | ElementFlag::MOVABLE);
      SpannerSegment(const SpannerSegment&);
      virtual SpannerSegment* clone() const = 0;

      virtual qreal mag() const override;
      virtual Fraction tick() const override;

      Spanner* spanner() const              { return _spanner;            }
      Spanner* setSpanner(Spanner* val)     { return _spanner = val;      }

      void setSpannerSegmentType(SpannerSegmentType s) { _spannerSegmentType = s;               }
      SpannerSegmentType spannerSegmentType() const    { return _spannerSegmentType;            }
      bool isSingleType() const                        { return spannerSegmentType() == SpannerSegmentType::SINGLE; }
      bool isBeginType() const                         { return spannerSegmentType() == SpannerSegmentType::BEGIN;  }
      bool isSingleBeginType() const                   { return isSingleType() || isBeginType(); }
      bool isSingleEndType() const                     { return isSingleType() || isEndType(); }
      bool isMiddleType() const                        { return spannerSegmentType() == SpannerSegmentType::MIDDLE; }
      bool isEndType() const                           { return spannerSegmentType() == SpannerSegmentType::END;    }

      void setSystem(System* s);
      System* system() const                { return toSystem(parent()); }

      const QPointF& userOff2() const       { return _offset2;       }
      void setUserOff2(const QPointF& o)    { _offset2 = o;          }
      void setUserXoffset2(qreal x)         { _offset2.setX(x);      }
      qreal& rUserXoffset2()                { return _offset2.rx();  }
      qreal& rUserYoffset2()                { return _offset2.ry();  }

      void setPos2(const QPointF& p)        { _p2 = p;                }
      //TODO: rename to spanSegPosWithUserOffset()
      QPointF pos2() const                  { return _p2 + _offset2; }
      //TODO: rename to spanSegPos()
      const QPointF& ipos2() const          { return _p2;             }
      QPointF& rpos2()                      { return _p2;             }
      qreal& rxpos2()                       { return _p2.rx();        }
      qreal& rypos2()                       { return _p2.ry();        }

      virtual bool isEditable() const override { return true; }

      QByteArray mimeData(const QPointF& dragOffset) const override;

      virtual void spatiumChanged(qreal ov, qreal nv) override;

      virtual QVariant getProperty(Pid id) const override;
      virtual bool setProperty(Pid id, const QVariant& v) override;
      virtual QVariant propertyDefault(Pid id) const override;
      virtual Element* propertyDelegate(Pid) override;
      virtual void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;
      using ScoreElement::undoChangeProperty;

      virtual Sid getPropertyStyle(Pid id) const override;
      virtual PropertyFlags propertyFlags(Pid id) const override;
      virtual void resetProperty(Pid id) override;
      virtual void styleChanged() override;
      void reset() override;

      virtual void setSelected(bool f) override;
      virtual void setVisible(bool f) override;
      virtual void setColor(const QColor& col) override;

      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;
      virtual QString accessibleInfo() const override;
      virtual void triggerLayout() const override;
      void autoplaceSpannerSegment();
      };

//----------------------------------------------------------------------------------
//   @@ Spanner
///   Virtual base class for slurs, ties, lines etc.
//
//    @P anchor         enum (Spanner.CHORD, Spanner.MEASURE, Spanner.NOTE, Spanner.SEGMENT)
//    @P endElement     Element           the element the spanner end is anchored to (read-only)
//    @P startElement   Element           the element the spanner start is anchored to (read-only)
//    @P tick           int               tick start position
//    @P tick2          int               tick end position
//----------------------------------------------------------------------------------

class Spanner : public Element {
      Q_GADGET
   public:
      enum class Anchor {
            SEGMENT, MEASURE, CHORD, NOTE
            };
      Q_ENUM(Anchor);
   private:

      Element* _startElement { 0  };
      Element* _endElement   { 0  };

      Anchor _anchor         { Anchor::SEGMENT };
      Fraction _tick         { Fraction(-1, 1) };
      Fraction _ticks        { Fraction(0, 1) };
      int _track2            { -1 };
      bool _broken           { false };

      std::vector<SpannerSegment*> segments;
      std::deque<SpannerSegment*> unusedSegments; // Currently unused segments which can be reused later.
                                                  // We cannot just delete them as they can be referenced
                                                  // in undo stack or other places already.

   protected:
      void pushUnusedSegment(SpannerSegment* seg);
      SpannerSegment* popUnusedSegment();
      void reuse(SpannerSegment* seg);            // called when segment from unusedSegments
                                                  // is added back to the spanner.
      int reuseSegments(int number);
      SpannerSegment* getNextLayoutSystemSegment(System* system, std::function<SpannerSegment*()> createSegment);
      void fixupSegments(unsigned int targetNumber, std::function<SpannerSegment*()> createSegment);

      const std::vector<SpannerSegment*> spannerSegments() const { return segments; }

   public:
      Spanner(Score* s, ElementFlags = ElementFlag::NOTHING);
      Spanner(const Spanner&);
      ~Spanner();

      virtual qreal mag() const override;

      virtual ElementType type() const = 0;
      virtual void setScore(Score* s) override;

      bool readProperties(XmlReader&) override;
      void writeProperties(XmlWriter&) const override;

      void writeSpannerStart(XmlWriter& xml, const Element* current, int track, Fraction frac = { -1, 1 }) const;
      void writeSpannerEnd(XmlWriter& xml,   const Element* current, int track, Fraction frac = { -1, 1 }) const;
      static void readSpanner(XmlReader& e, Element* current, int track);
      static void readSpanner(XmlReader& e, Score* current, int track);

      virtual Fraction tick() const override { return _tick;          }
      Fraction tick2() const                 { return _tick + _ticks; }
      Fraction ticks() const                 { return _ticks;         }

      void setTick(const Fraction&);
      void setTick2(const Fraction&);
      void setTicks(const Fraction&);

      bool isVoiceSpecific() const;
      int track2() const       { return _track2;   }
      void setTrack2(int v)    { _track2 = v;      }
      int effectiveTrack2() const { return _track2 == -1 ? track() : _track2; }

      bool broken() const      { return _broken;   }
      void setBroken(bool v)   { _broken = v;      }

      Anchor anchor() const    { return _anchor;   }
      void setAnchor(Anchor a) { _anchor = a;      }

      const std::vector<SpannerSegment*>& spannerSegments() { return segments; }
      SpannerSegment* frontSegment()               { return segments.front(); }
      const SpannerSegment* frontSegment() const   { return segments.front(); }
      SpannerSegment* backSegment()                { return segments.back();  }
      const SpannerSegment* backSegment() const    { return segments.back();  }
      SpannerSegment* segmentAt(int n)             { return segments[n];      }
      const SpannerSegment* segmentAt(int n) const { return segments[n];      }
      size_t nsegments() const                     { return segments.size();  }
      bool segmentsEmpty() const                   { return segments.empty(); }
      void eraseSpannerSegments();

      virtual SpannerSegment* layoutSystem(System*);
      virtual void layoutSystemsDone();

      virtual void triggerLayout() const override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      bool removeSpannerBack();
      virtual void removeUnmanaged();
      virtual void insertTimeUnmanaged(const Fraction& tick, const Fraction& len);

      QVariant getProperty(Pid propertyId) const;
      bool setProperty(Pid propertyId, const QVariant& v);
      QVariant propertyDefault(Pid propertyId) const;
      virtual void undoChangeProperty(Pid id, const QVariant&, PropertyFlags ps) override;

      void computeStartElement();
      void computeEndElement();
      static Note* endElementFromSpanner(Spanner* sp, Element* newStart);
      static Note* startElementFromSpanner(Spanner* sp, Element* newEnd);
      void setNoteSpan(Note* startNote, Note* endNote);

      Element* startElement() const    { return _startElement; }
      Element* endElement() const      { return _endElement;   }

      Measure* startMeasure() const;
      Measure* endMeasure() const;

      void setStartElement(Element* e);
      void setEndElement(Element* e);

      ChordRest* startCR();
      ChordRest* endCR();

      Chord* startChord();
      Chord* endChord();

      Segment* startSegment() const;
      Segment* endSegment() const;

      virtual void setSelected(bool f) override;
      virtual void setVisible(bool f) override;
      virtual void setAutoplace(bool f) override;
      virtual void setColor(const QColor& col) override;
      Spanner* nextSpanner(Element* e, int activeStaff);
      Spanner* prevSpanner(Element* e, int activeStaff);
      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;

      friend class SpannerSegment;
      using ScoreElement::undoChangeProperty;
      };

}     // namespace Ms
#endif
