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
class Chord;
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

      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;
      virtual bool isEditable() const override { return true; }

      virtual QVariant getProperty(P_ID id) const override;
      virtual bool setProperty(P_ID id, const QVariant& v) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      virtual void reset() override;
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
      Q_PROPERTY(int    tick2   READ tick2   WRITE setTick2)
      Q_PROPERTY(Anchor anchor  READ anchor  WRITE setAnchor)

      QList<SpannerSegment*> segments;
      Anchor _anchor;
      Element* _startElement;
      Element* _endElement;
      int _tick, _tick2;
      int _track2 = -1;
      int _id;                // used for xml serialization

      static QList<QPointF> userOffsets;
      static QList<QPointF> userOffsets2;

   protected:
      static int editTick, editTick2, editTrack2;

   public:
      Spanner(Score* = 0);
      Spanner(const Spanner&);
      ~Spanner();

      virtual ElementType type() const = 0;
      virtual void setScore(Score* s) override;

      int tick() const         { return _tick;          }
      void setTick(int v)      { _tick = v;             }
      int tickLen() const      { return _tick2 - _tick; }
      int tick2() const        { return _tick2;         }
      void setTick2(int v)     { _tick2 = v;            }
      int track2() const       { return _track2;        }
      void setTrack2(int v)    { _track2 = v;           }

      int id() const           { return _id; }
      void setId(int v)        { _id = v;    }

      Anchor anchor() const    { return _anchor;   }
      void setAnchor(Anchor a) { _anchor = a;      }

      const QList<SpannerSegment*>& spannerSegments() const { return segments; }
      QList<SpannerSegment*>& spannerSegments()             { return segments; }

      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;
      virtual void setSelected(bool f) override;
      bool removeSpannerBack();
      virtual void setYoff(qreal) {};    // used in musicxml import

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant& v);
      QVariant propertyDefault(P_ID propertyId) const;

      void computeStartElement();
      void computeEndElement();

      Element* startElement() const    { return _startElement; }
      Element* endElement() const      { return _endElement;   }

      void setStartElement(Element* e) { _startElement = e; }
      void setEndElement(Element* e)   { _endElement = e; }

      void setStartChord(Chord*);
      void setEndChord(Chord*);

      Chord* startChord() const;
      Chord* endChord() const;

      ChordRest* startCR() const;
      ChordRest* endCR() const;

      Segment* startSegment() const;
      Segment* endSegment() const;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Spanner::Anchor)

#endif

