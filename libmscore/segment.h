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

class QPainter;

namespace Ms {

class Measure;
class Segment;
class ChordRest;
class Lyrics;
class Spanner;
class System;

//---------------------------------------------------------
//   SegmentType
//---------------------------------------------------------

enum class SegmentType : short {
      Invalid            = 0x0,
      Clef               = 0x1,        // type from SegClef to SegTimeSig
      KeySig             = 0x2,        // need to be in the order in which they
      Ambitus            = 0x4,        // appear in a measure
      TimeSig            = 0x8,
      StartRepeatBarLine = 0x10,
      BarLine            = 0x20,
      ChordRest          = 0x40,
      Breath             = 0x80,
      EndBarLine         = 0x100,
      TimeSigAnnounce    = 0x200,
      KeySigAnnounce     = 0x400,
      All                = -1
      };

constexpr SegmentType operator| (SegmentType t1, SegmentType t2) {
      return static_cast<SegmentType>(static_cast<short>(t1) | static_cast<short>(t2));
      }
constexpr bool operator& (SegmentType t1, SegmentType t2) {
      return static_cast<short>(t1) & static_cast<short>(t2);
      }

//------------------------------------------------------------------------
//   @@ Segment
///    A segment holds all vertical aligned staff elements.
///    Segments are typed and contain only Elements of the same type.
//
//   @P segmentType  Ms::SegmentType  (Invalid, Clef, KeySig, Ambitus, TimeSig, StartRepeatBarLine, BarLine, ChordRest, Breath, EndBarLine TimeSigAnnounce, KeySigAnnounce, All)
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
      Q_PROPERTY(Ms::SegmentType segmentType READ segmentType WRITE setSegmentType)

   private:
      Segment* _next;               // linked list of segments inside a measure
      Segment* _prev;

      mutable bool empty;           // cached value
      mutable bool _written;        // used for write()

      SegmentType _segmentType { SegmentType::Invalid };
      int _tick;
      Spatium _extraLeadingSpace;
      Spatium _extraTrailingSpace;
      QList<qreal>   _dotPosX;     ///< size = staves

      std::vector<Element*> _annotations;

      QList<Element*> _elist;      ///< Element storage, size = staves * VOICES.

      void init();
      void checkEmpty() const;
      void checkElement(Element*, int track);

   public:
      Segment(Measure* m = 0);
      Segment(Measure*, SegmentType, int tick);
      Segment(const Segment&);
      ~Segment();

      virtual Segment* clone() const    { return new Segment(*this); }
      virtual ElementType type() const  { return ElementType::SEGMENT; }

      virtual void setScore(Score*);

      Q_INVOKABLE Ms::Segment* next() const             { return _next;   }
      Segment* next(SegmentType) const;

      void setNext(Segment* e)          { _next = e;      }
      Q_INVOKABLE Ms::Segment* prev() const { return _prev;   }
      Segment* prev(SegmentType) const;
      void setPrev(Segment* e)          { _prev = e;      }

      Q_INVOKABLE Ms::Segment* next1() const;
      Ms::Segment* next1MM() const;
      Segment* next1(SegmentType) const;
      Segment* next1MM(SegmentType) const;
      Q_INVOKABLE Ms::Segment* prev1() const;
      Ms::Segment* prev1MM() const;
      Segment* prev1(SegmentType) const;
      Segment* prev1MM(SegmentType) const;

      Segment* nextCR(int track = -1) const;

      ChordRest* nextChordRest(int track, bool backwards = false) const;

      Q_INVOKABLE Ms::Element* element(int track) const { return _elist.value(track);  }
      ChordRest* cr(int track) const                    {
            Q_ASSERT(_segmentType == SegmentType::ChordRest);
            return (ChordRest*)(_elist.value(track));
            };
      const QList<Element*>& elist() const { return _elist; }
      QList<Element*>& elist()             { return _elist; }

      void removeElement(int track);
      void setElement(int track, Element* el);
      const QList<Lyrics*>* lyricsList(int track) const;

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
      static const char* subTypeName(SegmentType);
      static SegmentType segmentType(ElementType type);
      SegmentType segmentType() const            { return _segmentType; }
      void setSegmentType(SegmentType t);

      void removeGeneratedElements();
      bool isEmpty() const                       { return empty; }
      void fixStaffIdx();
      bool isChordRest() const                   { return _segmentType == SegmentType::ChordRest; }
      void setTick(int);
      int tick() const;
      int rtick() const                          { return _tick; } // tickposition relative to measure start
      void setRtick(int val)                     { _tick = val; }

      bool splitsTuplet() const;

      const std::vector<Element*>& annotations() const { return _annotations;        }
      void clearAnnotations();
      void removeAnnotation(Element* e);
      bool findAnnotationOrElement(ElementType type, int minTrack, int maxTrack);

      qreal dotPosX(int staffIdx) const          { return _dotPosX[staffIdx];  }
      void setDotPosX(int staffIdx, qreal val)   { _dotPosX[staffIdx] = val;   }

      Spatium extraLeadingSpace() const          { return _extraLeadingSpace;  }
      void setExtraLeadingSpace(Spatium v)       { _extraLeadingSpace = v;     }
      Spatium extraTrailingSpace() const         { return _extraTrailingSpace; }
      void setExtraTrailingSpace(Spatium v)      { _extraTrailingSpace = v;    }
      bool written() const                       { return _written; }
      void setWritten(bool val)                  { _written = val; }
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;

      bool operator<(const Segment&) const;
      bool operator>(const Segment&) const;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::SegmentType);

#endif

