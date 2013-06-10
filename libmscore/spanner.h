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

class Segment;
class Spanner;
class System;
class ChordRest;

//---------------------------------------------------------
//   SpannerSegmentType
//---------------------------------------------------------

enum SpannerSegmentType {
      SEGMENT_SINGLE, SEGMENT_BEGIN, SEGMENT_MIDDLE, SEGMENT_END
      };

//---------------------------------------------------------
//   @@ SpannerSegment
//    parent: System
//---------------------------------------------------------

class SpannerSegment : public Element {
      Q_OBJECT

      Spanner* _spanner;
      SpannerSegmentType _spannerSegmentType;

   protected:
      QPointF _p2;
      QPointF _userOff2;

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

      const QPointF& userOff2() const       { return _userOff2;       }
      void setUserOff2(const QPointF& o)    { _userOff2 = o;          }
      void setUserXoffset2(qreal x)         { _userOff2.setX(x);      }
      void setPos2(const QPointF& p)        { _p2 = p;                }
      QPointF pos2() const                  { return _p2 + _userOff2; }
      const QPointF& ipos2() const          { return _p2;             }

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
//    @P start     int     tick start position
//    @P length    int     length in ticks
//    @P anchor    Anchor  ANCHOR_SEGMENT ANCHOR_MEASURE ANCHOR_CHORD ANCHOR_NOTE
//----------------------------------------------------------------------------------

class Spanner : public Element {
      Q_OBJECT
      Q_ENUMS(Anchor)

   public:
      enum Anchor {
            ANCHOR_SEGMENT, ANCHOR_MEASURE, ANCHOR_CHORD, ANCHOR_NOTE
            };
   private:
      Q_PROPERTY(int    tick    READ tick    WRITE setTick)
      Q_PROPERTY(int    tickLen READ tickLen WRITE setTickLen)
      Q_PROPERTY(Anchor anchor  READ anchor  WRITE setAnchor)

      QList<SpannerSegment*> segments;
      Anchor _anchor;
      int _tick, _tickLen;
      int _id;                // used for xml serialization

      static int editTick, editTickLen;
      static QList<QPointF> userOffsets;
      static QList<QPointF> userOffsets2;

   public:
      Spanner(Score* = 0);
      Spanner(const Spanner&);
      ~Spanner();

      virtual ElementType type() const = 0;
      virtual void setScore(Score* s);

      int tick() const         { return _tick; }
      void setTick(int v)      { _tick = v;  }
      int tickLen() const      { return _tickLen; }
      void setTickLen(int v)   { _tickLen = v; }
      int tick2() const        { return _tick + _tickLen; }       // helper function
      void setTick2(int v)     { _tickLen = v - _tick;    }       // assume _tick is already set
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
      virtual void endEdit();
      virtual void setSelected(bool f);
      virtual bool isEdited(Spanner* originalSpanner) const;
      bool removeSpannerBack();
      virtual void setYoff(qreal) {};    // used in musicxml import

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant& v);
      QVariant propertyDefault(P_ID propertyId) const;

      ChordRest* startCR() const;      // helper functions
      ChordRest* endCR() const;
      Segment* startSegment() const;
      Segment* endSegment() const;

      Element* startElement() const;
      Element* endElement() const;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Spanner::Anchor)

#endif

