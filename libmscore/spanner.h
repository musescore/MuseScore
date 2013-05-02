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

class Segment;
class Spanner;
class System;

enum SpannerSegmentType {
      SEGMENT_SINGLE, SEGMENT_BEGIN, SEGMENT_MIDDLE, SEGMENT_END
      };

//---------------------------------------------------------
//   @@ SpannerSegment
//---------------------------------------------------------

class SpannerSegment : public Element {
      Q_OBJECT

      SpannerSegmentType _spannerSegmentType;
      Spanner* _spanner;

   public:
      SpannerSegment(Score* s);
      SpannerSegment(const SpannerSegment&);
      virtual SpannerSegment* clone() const = 0;
      Spanner* spanner() const              { return _spanner;            }
      Spanner* setSpanner(Spanner* val)     { return _spanner = val;      }

      void setSpannerSegmentType(SpannerSegmentType s) { _spannerSegmentType = s;               }
      SpannerSegmentType spannerSegmentType() const    { return _spannerSegmentType;            }

      void setSystem(System* s);
      System* system() const                { return (System*)parent();   }

      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void endEdit();
      virtual bool isEditable() const { return true; }
      virtual bool isEdited(SpannerSegment*) const = 0;

      virtual QVariant getProperty(P_ID id) const;
      virtual bool setProperty(P_ID id, const QVariant& v);
      virtual QVariant propertyDefault(P_ID id) const;
      };

//----------------------------------------------------------------------------------
//   @@ Spanner
///   Virtual base class for slurs, ties, lines etc.
//
//    @P startElement Element*
//    @P endElement Element*
//    @P anchor     Anchor ANCHOR_SEGMENT ANCHOR_MEASURE ANCHOR_CHORD ANCHOR_NOTE
//----------------------------------------------------------------------------------

class Spanner : public Element {
      Q_OBJECT
      Q_ENUMS(Anchor)

   public:
      enum Anchor {
            ANCHOR_SEGMENT, ANCHOR_MEASURE, ANCHOR_CHORD, ANCHOR_NOTE
            };
   private:
      Q_PROPERTY(Element* startElement READ startElement WRITE setStartElement)
      Q_PROPERTY(Element* endElement   READ endElement   WRITE setEndElement)
      Q_PROPERTY(Anchor   anchor       READ anchor       WRITE setAnchor)

      Spanner* _next;
      Element* _startElement;       // can be Note, ChordRest, Segment or Measure
      Element* _endElement;         // depending on anchor

      Anchor _anchor;    // enum { ANCHOR_SEGMENT, ANCHOR_MEASURE,
                         //        ANCHOR_CHORD, ANCHOR_NOTE};

      QList<SpannerSegment*> segments;

      int _tick1, _tick2;     // used for backward compatibility
      int _id;                // used for xml serialization

   protected:
      Element* oStartElement; // start/end element at startEdit()
      Element* oEndElement;

   public:
      Spanner(Score* = 0);
      Spanner(const Spanner&);
      ~Spanner();

      virtual ElementType type() const = 0;
      virtual void setScore(Score* s);

      void setStartElement(Element* e) { _startElement = e;    }
      void setEndElement(Element* e)   { _endElement = e;      }
      Element* startElement() const    { return _startElement; }
      Element* endElement() const      { return _endElement;   }

      //
      // used for backward compatibility:
      //
      void __setTick1(int v)   { _tick1 = v;    }
      void __setTick2(int v)   { _tick2 = v;    }
      int __tick1() const      { return _tick1; }
      int __tick2() const      { return _tick2; }

      int id() const           { return _id; }
      void setId(int v)        { _id = v;    }

      Anchor anchor() const    { return _anchor;   }
      void setAnchor(Anchor a) { _anchor = a;      }

      const QList<SpannerSegment*>& spannerSegments() const { return segments; }
      QList<SpannerSegment*>& spannerSegments()             { return segments; }

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual void setSelected(bool f);
      virtual bool isEdited(Spanner* originalSpanner) const;
      int startTick() const;
      int endTick() const;
      bool removeSpannerBack();
      void addSpannerBack();
      virtual void setYoff(qreal) {};    // used in musicxml import
      Spanner* next() const     { return _next; }
      void setNext(Spanner* sp) { _next = sp; }
      };

Q_DECLARE_METATYPE(Spanner::Anchor)
#endif

