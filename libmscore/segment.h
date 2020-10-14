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

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include "element.h"
#include "shape.h"
#include "mscore.h"

namespace Ms {

class Measure;
class Segment;
class ChordRest;
class Spanner;
class System;

//------------------------------------------------------------------------
//   @@ Segment
//    A segment holds all vertical aligned staff elements.
//    Segments are typed and contain only Elements of the same type.
//
//    All Elements in a segment start at the same tick. The Segment can store one Element for
//    each voice in each staff in the score.
//    Some elements (Clef, KeySig, TimeSig etc.) are assumed to always have voice zero
//    and can be found in _elist[staffIdx * VOICES];

//    Segments are children of Measures and store Clefs, KeySigs, TimeSigs,
//    BarLines and ChordRests.
//
//   @P annotations     array[Element]    the list of annotations (read only)
//   @P next            Segment           the next segment in the whole score; null at last score segment (read-only)
//   @P nextInMeasure   Segment           the next segment in measure; null at last measure segment (read-only)
//   @P prev            Segment           the previous segment in the whole score; null at first score segment (read-only)
//   @P prevInMeasure   Segment           the previous segment in measure; null at first measure segment (read-only)
//   @P segmentType     enum (Segment.All, .Ambitus, .BarLine, .Breath, .ChordRest, .Clef, .EndBarLine, .Invalid, .KeySig, .KeySigAnnounce, .StartRepeatBarLine, .TimeSig, .TimeSigAnnounce)
//   @P tick            int               midi tick position (read only)
//------------------------------------------------------------------------

class Segment final : public Element {
      SegmentType _segmentType { SegmentType::Invalid };
      Fraction _tick;  // { Fraction(0, 1) };
      Fraction _ticks; // { Fraction(0, 1) };
      Spatium _extraLeadingSpace;
      qreal _stretch;

      Segment* _next = nullptr;                     // linked list of segments inside a measure
      Segment* _prev = nullptr;

      std::vector<Element*> _annotations;
      std::vector<Element*> _elist;       // Element storage, size = staves * VOICES.
      std::vector<Shape>    _shapes;      // size = staves
      std::vector<qreal>    _dotPosX;     // size = staves


      void init();
      void checkEmpty() const;
      void checkElement(Element*, int track);
      void setEmpty(bool val) const { setFlag(ElementFlag::EMPTY, val); }

   protected:
      Element* getElement(int staff);     //??

   public:
      Segment(Measure* m = 0);
      Segment(Measure*, SegmentType, const Fraction&);
      Segment(const Segment&);
      ~Segment();

      Segment* clone() const override      { return new Segment(*this); }
      ElementType type() const override    { return ElementType::SEGMENT; }

      void setScore(Score*) override;

      Segment* next() const               { return _next;   }
      Segment* next(SegmentType) const;
      Segment* nextActive() const;
      Segment* nextEnabled() const;
      Segment* nextInStaff(int staffIdx, SegmentType t = SegmentType::ChordRest) const;
      void setNext(Segment* e)            { _next = e;      }

      Segment* prev() const               { return _prev;   }
      Segment* prev(SegmentType) const;
      Segment* prevActive() const;
      Segment* prevEnabled() const;
      void setPrev(Segment* e)            { _prev = e;      }

      // don’t stop at measure boundary:
      Segment* next1() const;
      Segment* next1enabled() const;
      Segment* next1MM() const;
      Segment* next1MMenabled() const;
      Segment* next1(SegmentType) const;
      Segment* next1MM(SegmentType) const;

      Segment* prev1() const;
      Segment* prev1enabled() const;
      Segment* prev1MM() const;
      Segment* prev1MMenabled() const;
      Segment* prev1(SegmentType) const;
      Segment* prev1MM(SegmentType) const;

      Segment* nextCR(int track = -1, bool sameStaff = false) const;

      ChordRest* nextChordRest(int track, bool backwards = false) const;

      Element* element(int track) const;

      // a variant of the above function, specifically designed to be called from QML
      //@ returns the element at track 'track' (null if none)
      Ms::Element* elementAt(int track) const;

      const std::vector<Element*>& elist() const { return _elist; }
      std::vector<Element*>& elist()             { return _elist; }

      void removeElement(int track);
      void setElement(int track, Element* el);
      void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;

      Measure* measure() const                   { return toMeasure(parent()); }
      System* system() const                     { return toSystem(parent()->parent()); }
      qreal x() const override                   { return ipos().x();         }
      void setX(qreal v)                         { rxpos() = v;               }

      void insertStaff(int staff);
      void removeStaff(int staff);

      void add(Element*) override;
      void remove(Element*) override;
      void swapElements(int i1, int i2);

      void sortStaves(QList<int>& dst);
      const char* subTypeName() const;

      static const char* subTypeName(SegmentType);
      static SegmentType segmentType(ElementType type);

      SegmentType segmentType() const            { return _segmentType; }
      void setSegmentType(SegmentType t);

      bool empty() const                         { return flag(ElementFlag::EMPTY); }
      bool written() const                       { return flag(ElementFlag::WRITTEN); }
      void setWritten(bool val) const            { setFlag(ElementFlag::WRITTEN, val); }

      void fixStaffIdx();

      qreal stretch() const                      { return _stretch; }
      void setStretch(qreal v)                   { _stretch = v;    }

      Fraction rtick() const override    { return _tick;    }
      void setRtick(const Fraction& v)           { Q_ASSERT(v >= Fraction(0,1));  _tick = v;       }
      Fraction tick() const override;

      Fraction ticks() const                     { return _ticks;   }
      void setTicks(const Fraction& v)           { _ticks = v;      }

      qreal widthInStaff(int staffIdx, SegmentType t = SegmentType::ChordRest) const;
      Fraction ticksInStaff(int staffIdx) const;

      bool splitsTuplet() const;

      const std::vector<Element*>& annotations() const { return _annotations;        }
      void clearAnnotations();
      void removeAnnotation(Element* e);
      bool hasAnnotationOrElement(ElementType type, int minTrack, int maxTrack) const;
      Element* findAnnotation(ElementType type, int minTrack, int maxTrack);
      std::vector<Element*> findAnnotations(ElementType type, int minTrack, int maxTrack);
      bool hasElements() const;
      bool hasElements(int minTrack, int maxTrack) const;

      qreal dotPosX(int staffIdx) const          { return _dotPosX[staffIdx];  }
      void setDotPosX(int staffIdx, qreal val)   { _dotPosX[staffIdx] = val;   }

      Spatium extraLeadingSpace() const          { return _extraLeadingSpace;  }
      void setExtraLeadingSpace(Spatium v)       { _extraLeadingSpace = v;     }

      void write(XmlWriter&) const override;
      void read(XmlReader&) override;

      QVariant getProperty(Pid propertyId) const override;
      bool setProperty(Pid propertyId, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

      bool operator<(const Segment&) const;
      bool operator>(const Segment&) const;

      virtual QString accessibleExtraInfo() const override;

      Element* firstInNextSegments(int activeStaff); //<
      Element* lastInPrevSegments(int activeStaff);   //<
      Element* firstElement(int staff);              //<  These methods are used for navigation
      Element* lastElement(int staff);               //<  for next-element and prev-element
      Element* firstElementOfSegment(Segment* s, int activeStaff);
      Element* nextElementOfSegment(Segment* s, Element* e, int activeStaff);
      Element* prevElementOfSegment(Segment* s, Element* e, int activeStaff);
      Element* lastElementOfSegment(Segment* s, int activeStaff);
      Element* nextAnnotation(Element* e);
      Element* prevAnnotation(Element* e);
      Element* firstAnnotation(Segment* s, int activeStaff);
      Element* lastAnnotation(Segment* s, int activeStaff);
      Spanner* firstSpanner(int activeStaff);
      Spanner* lastSpanner(int activeStaff);
      bool notChordRestType(Segment* s);
      using Element::nextElement;
      Element* nextElement(int activeStaff);
      using Element::prevElement;
      Element* prevElement(int activeStaff);

      std::vector<Shape> shapes()                     { return _shapes; }
      const std::vector<Shape>& shapes() const        { return _shapes; }
      const Shape& staffShape(int staffIdx) const     { return _shapes[staffIdx]; }
      Shape& staffShape(int staffIdx)                 { return _shapes[staffIdx]; }
      void createShapes();
      void createShape(int staffIdx);
      qreal minRight() const;
      qreal minLeft(const Shape&) const;
      qreal minLeft() const;
      qreal minHorizontalDistance(Segment*, bool isSystemGap) const;
      qreal minHorizontalCollidingDistance(Segment* ns) const;

      // some helper function
      ChordRest* cr(int track) const        { return toChordRest(_elist[track]); }
      bool isType(const SegmentType t) const{ return int(_segmentType) & int(t); }
      bool isBeginBarLineType() const       { return _segmentType == SegmentType::BeginBarLine; }
      bool isClefType() const               { return _segmentType == SegmentType::Clef; }
      bool isHeaderClefType() const         { return _segmentType == SegmentType::HeaderClef; }
      bool isKeySigType() const             { return _segmentType == SegmentType::KeySig; }
      bool isAmbitusType() const            { return _segmentType == SegmentType::Ambitus; }
      bool isTimeSigType() const            { return _segmentType == SegmentType::TimeSig; }
      bool isStartRepeatBarLineType() const { return _segmentType == SegmentType::StartRepeatBarLine; }
      bool isBarLineType() const            { return _segmentType == SegmentType::BarLine; }
      bool isBreathType() const             { return _segmentType == SegmentType::Breath; }
      bool isChordRestType() const          { return _segmentType == SegmentType::ChordRest; }
      bool isEndBarLineType() const         { return _segmentType == SegmentType::EndBarLine; }
      bool isKeySigAnnounceType() const     { return _segmentType == SegmentType::KeySigAnnounce; }
      bool isTimeSigAnnounceType() const    { return _segmentType == SegmentType::TimeSigAnnounce; }

      static constexpr SegmentType durationSegmentsMask = SegmentType::ChordRest; // segment types which may have non-zero tick length
      };

//---------------------------------------------------------
//   nextActive
//---------------------------------------------------------

inline Segment* Segment::nextActive() const
      {
      Segment* ns = next();
      while (ns && !(ns->enabled() && ns->visible()))
            ns = ns->next();
      return ns;
      }

//---------------------------------------------------------
//   nextEnabled
//---------------------------------------------------------

inline Segment* Segment::nextEnabled() const
      {
      Segment* ns = next();
      while (ns && !ns->enabled())
            ns = ns->next();
      return ns;
      }

//---------------------------------------------------------
//   prevActive
//---------------------------------------------------------

inline Segment* Segment::prevActive() const
      {
      Segment* ps = prev();
      while (ps && !(ps->enabled() && ps->visible()))
            ps = ps->prev();
      return ps;
      }

//---------------------------------------------------------
//   prevEnabled
//---------------------------------------------------------

inline Segment* Segment::prevEnabled() const
      {
      Segment* ps = prev();
      while (ps && !ps->enabled())
            ps = ps->prev();
      return ps;
      }

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::SegmentType);

#endif

