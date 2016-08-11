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
      Q_PROPERTY(Spatium                  borderWidth      READ borderWidth)
      Q_PROPERTY(int                      opacity          READ opacity)
      Score* _score;
      int _opacity;
      int _staffStart;
      int _staffEnd;
      qreal _boxHeight;
      Segment* _startSegment;
      Segment* _endSegment;
      Spatium _borderWidth  { Spatium(0) };
      qreal _leftMargin { 0.0 }, _rightMargin { 0.0 };
      qreal _topMargin  { 0.0 }, _bottomMargin { 0.0 };

   public:

      RangeAnnotation(Score*  = 0);
      virtual RangeAnnotation* clone() const override         { return new RangeAnnotation(*this); }
      virtual Element::Type type() const                 { return Element::Type::RANGEANNOTATION; }

      Score* score() const                { return _score; }
      void setStaffStart(int v)           { _staffStart = v; }
      void setStaffEnd(int v)             { _staffEnd = v; }
      int staffStart()                    { return _staffStart; }
      int staffEnd()                      { return _staffEnd;   }
      qreal boxHeight()                   { return _boxHeight;  }
      void setBoxHeight(qreal v)                 { _boxHeight = v;     }
      void setStartSegment(Segment* s)           { _startSegment = s; }
      void setEndSegment(Segment* s)             { _endSegment = s; }
      Segment* startSegment()                    { return _startSegment; }
      Segment* endSegment()                      { return _endSegment;   }
      qreal firstNoteRestSegmentX(System* system);
      virtual RangeAnnotationSegment* layoutSystem(System* system);
      void rangePos(RangePos*);
      Spatium borderWidth()                { return _borderWidth; }
      int opacity()                    { return _opacity;     }
   //   void setBorderWidth(int v);
   //   void setOpacity(int v);

      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void writeProperties(Xml&) const override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      friend class RangeAnnotationSegment;

      };


}     // namespace Ms




#endif // RANGEANNOTATION_H
