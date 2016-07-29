#ifndef RANGEANNOTATION_H
#define RANGEANNOTATION_H


/**
 \file
 Definition of class RangeAnnotation
*/
#include "spanner.h"

namespace Ms {

class Score;
struct RangePos;

//---------------------------------------------------------
//   RangePos
//---------------------------------------------------------

struct RangePos {
      QPointF p1;             // start point of the range annotation
      System* system1;        // start system of the range annotation
      QPointF p2;             // end point of the range annotation
      System* system2;        // end system of the range annotation
      };

//---------------------------------------------------------
//   @@ RangeAnnotationSegment
//    a single segment of RangeAnnotation
//---------------------------------------------------------

class RangeAnnotationSegment : public SpannerSegment {
      Q_OBJECT
      QPainterPath path;

   public:
      RangeAnnotationSegment(Score*);
      virtual RangeAnnotationSegment* clone() const { return new RangeAnnotationSegment(*this); }
      virtual Element::Type type() const { return Element::Type::RANGEANNOTATION_SEGMENT; }
      virtual int subtype() const         { return static_cast<int>(spanner()->type()); }
      virtual QString subtypeName() const { return name(spanner()->type()); }

      void layoutSegment(const QPointF& p1, const QPointF& p2);
      RangeAnnotationSegment* layoutSystem(System* system);
      virtual void draw(QPainter*) const;
      friend class RangeAnnotation;
      };

//-------------------------------------- -------------------
//   @@ RangeAnnotation
//---------------------------------------------------------

class RangeAnnotation : public Spanner {
      Q_OBJECT
   /*   Q_PROPERTY(Ms::Segment*             startSegment        READ startSegment)
      Q_PROPERTY(Ms::Segment*             endSegment          READ endSegment)
      Q_PROPERTY(int                      staffStart          READ staffStart)
      Q_PROPERTY(int                      staffEnd            READ staffEnd)*/

      Score* _score;
      int _staffStart;
      int _staffEnd;
      Segment* _startSegment;
      Segment* _endSegment;

   public:

      RangeAnnotation(Score*  = 0);
      virtual RangeAnnotation* clone() const override         { return new RangeAnnotation(*this); }
      virtual Element::Type type() const                 { return Element::Type::RANGEANNOTATION; }

      Score* score() const             { return _score; }
      qreal firstNoteRestSegmentX(System* system);
      virtual RangeAnnotationSegment* layoutSystem(System* system);
    /*  Segment* startSegment() const     { return _startSegment; }
      Segment* endSegment() const       { return _endSegment;   }
      void setStartSegment(Segment* s)  { _startSegment = s; }
      void setEndSegment(Segment* s)    { _endSegment = s; }
      void setRange(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd);
      int tickStart() const;
      int tickEnd() const;
      int staffStart() const            { return _staffStart;  }
      int staffEnd() const              { return _staffEnd;    }
      void setStaffStart(int v)         { _staffStart = v;  }
      void setStaffEnd(int v)           { _staffEnd = v;    }*/
      void rangePos(RangePos*);
 //     virtual void write(Xml& xml) const override;
 //     virtual void read(XmlReader&) override;
 //     virtual void layout() override;
      friend class RangeAnnotationSegment;
      };


}     // namespace Ms




#endif // RANGEANNOTATION_H
