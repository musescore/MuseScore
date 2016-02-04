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

/**
 \file
 Definition of class Segment.
*/

#ifndef __SEGMENT_H__
#define __SEGMENT_H__

#include "element.h"
#include "shape.h"
class QPainter;

namespace Ms {

class Measure;
class Segment;
class ChordRest;
class Lyrics;
class Spanner;
class System;

//------------------------------------------------------------------------
//   @@ Segment
///    A segment holds all vertical aligned staff elements.
///    Segments are typed and contain only Elements of the same type.
//
//   @P annotations     array[Element]    the list of annotations (read only)
//   @P next            Segment           the next segment in the whole score; null at last score segment (read-only)
//   @P nextInMeasure   Segment           the next segment in measure; null at last measure segment (read-only)
//   @P prev            Segment           the previous segment in the whole score; null at first score segment (read-only)
//   @P prevInMeasure   Segment           the previous segment in measure; null at first measure segment (read-only)
//   @P segmentType     enum (Segment.All, .Ambitus, .BarLine, .Breath, .ChordRest, .Clef, .EndBarLine, .Invalid, .KeySig, .KeySigAnnounce, .StartRepeatBarLine, .TimeSig, .TimeSigAnnounce)
//   @P tick            int               midi tick position (read only)
//------------------------------------------------------------------------

/**
 All Elements in a segment start at the same tick. The Segment can store one Element for
 each voice in each staff in the score. It also stores the lyrics for each staff.
 Some elements (Clef, KeySig, TimeSig etc.) are assumed to always have voice zero
 and can be found in _elist[staffIdx * VOICES];

 Segments are children of Measures and store Clefs, KeySigs, TimeSigs,
 BarLines and ChordRests.
*/

class Segment : public Element {
      Q_OBJECT
      Q_PROPERTY(QQmlListProperty<Ms::Element> annotations READ qmlAnnotations)
      Q_PROPERTY(Ms::Segment*       next              READ next1)
      Q_PROPERTY(Ms::Segment*       nextInMeasure     READ next)
      Q_PROPERTY(Ms::Segment*       prev              READ prev1)
      Q_PROPERTY(Ms::Segment*       prevInMeasure     READ prev)
      Q_PROPERTY(Ms::Segment::Type  segmentType       READ segmentType WRITE setSegmentType)
      Q_PROPERTY(int                tick              READ tick)
      Q_ENUMS(Type)

public:
   enum class Type {
         Invalid            = 0x0,
         BeginBarLine       = 0x1,
         Clef               = 0x2,        // type from Clef to TimeSig
         KeySig             = 0x4,        // need to be in the order in which they
         Ambitus            = 0x8,        // appear in a measure
         TimeSig            = 0x10,
         StartRepeatBarLine = 0x20,
         BarLine            = 0x40,
         Breath             = 0x80,
         ChordRest          = 0x100,
         EndBarLine         = 0x200,
         KeySigAnnounce     = 0x400,
         TimeSigAnnounce    = 0x800,
         All                = -1
         };

   private:
      Segment* _next;                     // linked list of segments inside a measure
      Segment* _prev;

      mutable bool empty;                 // cached value
      mutable bool _written { false };    // used for write()

      Type _segmentType { Type::Invalid };
      int _tick;
      int _ticks;
      Spatium _extraLeadingSpace;

      QList<qreal>   _dotPosX;            ///< size = staves
      std::vector<Element*> _annotations;
      QList<Element*> _elist;             ///< Element storage, size = staves * VOICES.
      QList<Shape>    _shapes;            // size = staves

      void init();
      void checkEmpty() const;
      void checkElement(Element*, int track);

   protected:
      Element* getElement(int staff);     //??

   public:
      Segment(Measure* m = 0);
      Segment(Measure*, Type, int tick);
      Segment(const Segment&);
      ~Segment();

      virtual Segment* clone() const     { return new Segment(*this); }
      virtual Element::Type type() const { return Element::Type::SEGMENT; }

      virtual void setScore(Score*);

      Ms::Segment* next() const          { return _next;   }
      Segment* next(Type) const;

      void setNext(Segment* e)           { _next = e;      }
      Ms::Segment* prev() const          { return _prev;   }
      Segment* prev(Type) const;
      void setPrev(Segment* e)           { _prev = e;      }

      Ms::Segment* next1() const;
      Ms::Segment* next1MM() const;
      Segment* next1(Type) const;
      Segment* next1MM(Type) const;
      Ms::Segment* prev1() const;
      Ms::Segment* prev1MM() const;
      Segment* prev1(Type) const;
      Segment* prev1MM(Type) const;

      Segment* nextCR(int track = -1, bool sameStaff = false) const;

      ChordRest* nextChordRest(int track, bool backwards = false) const;

      Ms::Element* element(int track) const { return _elist.value(track);  }
      // a variant of the above function, specifically designed to be called from QML
      //@ returns the element at track 'track' (null if none)
      Q_INVOKABLE Ms::Element* elementAt(int track) const;
      const QList<Element*>& elist() const { return _elist; }
      QList<Element*>& elist()             { return _elist; }

      void removeElement(int track);
      void setElement(int track, Element* el);
      const QList<Lyrics*>* lyricsList(int track) const;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      Measure* measure() const            { return (Measure*)parent(); }
      System* system() const              { return (System*)parent()->parent(); }
      qreal x() const                     { return ipos().x();         }
      void setX(qreal v)                  { rxpos() = v;               }

      void insertStaff(int staff);
      void removeStaff(int staff);

      virtual void add(Element*);
      virtual void remove(Element*);
      void swapElements(int i1, int i2);

      void sortStaves(QList<int>& dst);
      const char* subTypeName() const;
      static const char* subTypeName(Type);
      static Type segmentType(Element::Type type);
      Type segmentType() const                   { return _segmentType; }
      void setSegmentType(Type t);

      void removeGeneratedElements();
      bool isEmpty() const                       { return empty; }
      void fixStaffIdx();
      void setTick(int);
      int tick() const;
      int rtick() const                          { return _tick; } // tickposition relative to measure start
      void setRtick(int val)                     { _tick = val; }
      int ticks() const                          { return _ticks; }
      void setTicks(int val)                     { _ticks = val; }

      bool splitsTuplet() const;

      const std::vector<Element*>& annotations() const { return _annotations;        }
      void clearAnnotations();
      void removeAnnotation(Element* e);
      bool findAnnotationOrElement(Element::Type type, int minTrack, int maxTrack);

      QQmlListProperty<Ms::Element> qmlAnnotations();

      qreal dotPosX(int staffIdx) const          { return _dotPosX[staffIdx];  }
      void setDotPosX(int staffIdx, qreal val)   { _dotPosX[staffIdx] = val;   }

      Spatium extraLeadingSpace() const          { return _extraLeadingSpace;  }
      void setExtraLeadingSpace(Spatium v)       { _extraLeadingSpace = v;     }
      bool written() const                       { return _written; }
      void setWritten(bool val)                  { _written = val; }
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;

      bool operator<(const Segment&) const;
      bool operator>(const Segment&) const;

      virtual QString accessibleExtraInfo() const override;
      Element* firstInNextSegments(int activeStaff); //<
      Element* lastInPrevSegments(int activeStaff);   //<
      Element* firstElement(int staff);              //<  These methods are used for navigation
      Element* lastElement(int staff);               //<  for next-element and prev-element

      QList<Shape> shapes()              { return _shapes; }
      const QList<Shape>& shapes() const { return _shapes; }
      const Shape& shape(int i) const    { return _shapes[i]; }
      void createShapes();
      void createShape(int staffIdx);
      qreal minRight() const;
      qreal minLeft() const;
      qreal minHorizontalDistance(Segment* ns) const;

      // some helper function
      ChordRest* cr(int track) const                    {
            Q_ASSERT(_segmentType == Type::ChordRest);
            return (ChordRest*)(_elist[track]);
            };
      bool isBeginBarLine() const       { return _segmentType == Type::BeginBarLine; }
      bool isClef() const               { return _segmentType == Type::Clef; }
      bool isKeySig() const             { return _segmentType == Type::KeySig; }
      bool isAmbitus() const            { return _segmentType == Type::Ambitus; }
      bool isTimeSig() const            { return _segmentType == Type::TimeSig; }
      bool isStartRepeatBarLine() const { return _segmentType == Type::StartRepeatBarLine; }
      bool isBarLine() const            { return _segmentType == Type::BarLine; }
      bool isBreath() const             { return _segmentType == Type::Breath; }
      bool isChordRest() const          { return _segmentType == Type::ChordRest; }
      bool isEndBarLine() const         { return _segmentType == Type::EndBarLine; }
      bool isKeySigAnnounce() const     { return _segmentType == Type::KeySigAnnounce; }
      bool isTimeSigAnnounce() const    { return _segmentType == Type::TimeSigAnnounce; }
      };

constexpr Segment::Type operator| (Segment::Type t1, Segment::Type t2) {
      return static_cast<Segment::Type>(static_cast<int>(t1) | static_cast<int>(t2));
      }
constexpr bool operator& (Segment::Type t1, Segment::Type t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Segment::Type);

#endif

