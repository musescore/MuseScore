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
     // Q_PROPERTY(int                      borderWidth          READ borderWidth)
     // Q_PROPERTY(QColor                   color            READ color)
     // Q_PROPERTY(int                      opacity          READ opacity)

      Score* _score;
    //  int _borderWidth;
    //  int _opacity;
    //  QColor _color;

   public:

      RangeAnnotation(Score*  = 0);
      virtual RangeAnnotation* clone() const override         { return new RangeAnnotation(*this); }
      virtual Element::Type type() const                 { return Element::Type::RANGEANNOTATION; }

      Score* score() const             { return _score; }
      qreal firstNoteRestSegmentX(System* system);
      virtual RangeAnnotationSegment* layoutSystem(System* system);
      void rangePos(RangePos*);
    //  QColor color() const     { return _color; }

 //     virtual void write(Xml& xml) const override;
 //     virtual void read(XmlReader&) override;
      friend class RangeAnnotationSegment;
      };


}     // namespace Ms




#endif // RANGEANNOTATION_H
