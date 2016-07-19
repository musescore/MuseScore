#ifndef RANGEANNOTATION_H
#define RANGEANNOTATION_H


/**
 \file
 Definition of class RangeAnnotation
*/
#include "text.h"
#include "textannotation.h"
namespace Ms {

class Score;
//---------------------------------------------------------
//   @@ Annotation
//---------------------------------------------------------

class RangeAnnotation : public Text {
      Q_OBJECT
      Score* _score;
      int _staffStart;
      int _staffEnd;
      Segment* _startSegment;
      Segment* _endSegment;

   public:

      RangeAnnotation(Score*  = 0);
      virtual RangeAnnotation* clone() const override         { return new RangeAnnotation(*this); }
      virtual Element::Type type() const                 { return Element::Type::ANNOTATION; }

      Score* score() const             { return _score; }
//      void drawRectangle(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd);
      Segment* startSegment() const     { return _startSegment; }
      Segment* endSegment() const       { return _endSegment;   }
      void setStartSegment(Segment* s)  { _startSegment = s; }
      void setEndSegment(Segment* s)    { _endSegment = s; }
      void setRange(Segment* startSegment, Segment* endSegment, int staffStart, int staffEnd);
      int tickStart() const;
      int tickEnd() const;
      int staffStart() const            { return _staffStart;  }
      int staffEnd() const              { return _staffEnd;    }
      void setStaffStart(int v)         { _staffStart = v;  }
      void setStaffEnd(int v)           { _staffEnd = v;    }
      };


}     // namespace Ms




#endif // RANGEANNOTATION_H
