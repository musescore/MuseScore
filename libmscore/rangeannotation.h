#ifndef RANGEANNOTATION_H
#define RANGEANNOTATION_H


/**
 \file
 Definition of class RangeAnnotation
*/
#include "spanner.h"

namespace Ms {

class Score;
class RangeAnnotation;
struct RangePos;

//---------------------------------------------------------
//   RangePos
//---------------------------------------------------------

struct RangePos {
      QPointF p1;             // start point of the range annotation
      System* system1;        // start system of the range annotation
      QPointF p2;             // end point of the range annotation
      System* system2;        // end system of the range annotation
      qreal height;
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
      void layoutSegment(RangePos* rp, RangeAnnotation* range);
      RangeAnnotationSegment* layoutSystem(System* system);
      virtual void writeProperties(Xml&) const override;
      virtual bool readProperties(XmlReader&) override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      virtual void draw(QPainter*) const;
      friend class RangeAnnotation;
      };

//-------------------------------------- -------------------
//   @@ RangeAnnotation
//---------------------------------------------------------

class RangeAnnotation : public Spanner {
      Q_OBJECT
      Score* _score;
      int _staffStart;
      int _staffEnd;
      Segment* _startSegment;
      Segment* _endSegment;
      Spatium _borderWidth  { Spatium(0) };
      qreal _leftMargin { 3.0 }, _rightMargin  { -3.0 };
      qreal _topMargin  { 1.0 }, _bottomMargin { 1.0 };

   public:

      RangeAnnotation(Score*  = 0);
      virtual RangeAnnotation* clone() const override  { return new RangeAnnotation(*this); }
      virtual Element::Type type() const               { return Element::Type::RANGEANNOTATION; }

      Score* score() const                             { return _score; }
      void setStaffStart(int v)                        { _staffStart = v; }
      void setStaffEnd(int v)                          { _staffEnd = v; }
      int staffStart() const                           { return _staffStart; }
      int staffEnd()  const                            { return _staffEnd;   }
      void setStartSegment(Segment* s)                 { _startSegment = s; }
      void setEndSegment(Segment* s)                   { _endSegment = s; }
      Segment* startSegment()                          { return _startSegment; }
      Segment* endSegment()                            { return _endSegment;   }
      qreal firstNoteRestSegmentX(System* system);
      virtual RangeAnnotationSegment* layoutSystem(System* system);
      void rangePos(RangePos* rp, SpannerSegmentType sst, System* system);
      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void writeProperties(Xml&) const override;
      virtual bool readProperties(XmlReader&) override;

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      friend class RangeAnnotationSegment;

      };


}     // namespace Ms




#endif // RANGEANNOTATION_H
