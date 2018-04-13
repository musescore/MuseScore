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

//-------------------------------------------------------------------
//   SegmentType
//
//    Type values determine the order of segments for a given tick
//-------------------------------------------------------------------

enum class SegmentType {
      Invalid            = 0x0,
      BeginBarLine       = 0x1,
      HeaderClef         = 0x2,
      KeySig             = 0x4,
      Ambitus            = 0x8,
      TimeSig            = 0x10,
      Clef               = 0x20,
      StartRepeatBarLine = 0x40,
      BarLine            = 0x80,
      Breath             = 0x100,
      //--
      ChordRest          = 0x200,
      //--
      EndBarLine         = 0x400,
      KeySigAnnounce     = 0x800,
      TimeSigAnnounce    = 0x1000,
      All                = -1,
      BarLineType        = BeginBarLine | StartRepeatBarLine | BarLine | EndBarLine
      };

constexpr SegmentType operator| (const SegmentType t1, const SegmentType t2) {
      return static_cast<SegmentType>(static_cast<int>(t1) | static_cast<int>(t2));
      }
constexpr bool operator& (const SegmentType t1, const SegmentType t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }

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
      int _tick = 0;                          // tick offset to measure
      int _ticks = 0;
      Spatium _extraLeadingSpace;
      qreal _stretch;

      Segment* _next;                     // linked list of segments inside a measure
      Segment* _prev;

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
      Segment(Measure*, SegmentType, int tick);
      Segment(const Segment&);
      ~Segment();

      virtual Segment* clone() const      { return new Segment(*this); }
      virtual ElementType type() const    { return ElementType::SEGMENT; }

      virtual void setScore(Score*);

      Segment* next() const               { return _next;   }
      Segment* next(SegmentType) const;
      Segment* nextEnabled() const;
      void setNext(Segment* e)            { _next = e;      }

      Segment* prev() const               { return _prev;   }
      Segment* prev(SegmentType) const;
      Segment* prevEnabled() const;
      void setPrev(Segment* e)            { _prev = e;      }

      // dont stop at measure boundary:
      Segment* next1() const;
      Segment* next1enabled() const;
      Segment* next1MM() const;
      Segment* next1(SegmentType) const;
      Segment* next1MM(SegmentType) const;

      Segment* prev1() const;
      Segment* prev1MM() const;
      Segment* prev1(SegmentType) const;
      Segment* prev1MM(SegmentType) const;

      Segment* nextCR(int track = -1, bool sameStaff = false) const;

      ChordRest* nextChordRest(int track, bool backwards = false) const;

      Element* element(int track) const { return _elist[track];  }

      // a variant of the above function, specifically designed to be called from QML
      //@ returns the element at track 'track' (null if none)
      Q_INVOKABLE Ms::Element* elementAt(int track) const;

      const std::vector<Element*>& elist() const { return _elist; }
      std::vector<Element*>& elist()             { return _elist; }

      void removeElement(int track);
      void setElement(int track, Element* el);
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      Measure* measure() const                   { return toMeasure(parent()); }
      System* system() const                     { return toSystem(parent()->parent()); }
      qreal x() const                            { return ipos().x();         }
      void setX(qreal v)                         { rxpos() = v;               }

      void insertStaff(int staff);
      void removeStaff(int staff);

      virtual void add(Element*);
      virtual void remove(Element*);
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
      void setTick(int t)                        { _tick = t - parent()->tick(); }
      virtual int tick() const override          { return _tick + parent()->tick(); }
      virtual int rtick() const override         { return _tick;  } // tickposition relative to measure start
      Fraction fpos() const;
      void setRtick(int val)                     { _tick = val;   }
      int ticks() const                          { return _ticks; }
      void setTicks(int val)                     { _ticks = val;  }

      bool splitsTuplet() const;

      const std::vector<Element*>& annotations() const { return _annotations;        }
      void clearAnnotations();
      void removeAnnotation(Element* e);
      bool findAnnotationOrElement(ElementType type, int minTrack, int maxTrack);
      bool findAnnotation(ElementType type, int minTrack, int maxTrack);


      qreal dotPosX(int staffIdx) const          { return _dotPosX[staffIdx];  }
      void setDotPosX(int staffIdx, qreal val)   { _dotPosX[staffIdx] = val;   }

      Spatium extraLeadingSpace() const          { return _extraLeadingSpace;  }
      void setExtraLeadingSpace(Spatium v)       { _extraLeadingSpace = v;     }

      virtual void write(XmlWriter&) const;
      virtual void read(XmlReader&);

      virtual QVariant getProperty(Pid propertyId) const;
      virtual bool setProperty(Pid propertyId, const QVariant&);
      virtual QVariant propertyDefault(Pid) const;

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
      };

//---------------------------------------------------------
//   nextEnabled
//---------------------------------------------------------

inline Segment* Segment::nextEnabled() const
      {
      Segment* ps = next();
      while (ps && !ps->enabled())
            ps = ps->next();
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

