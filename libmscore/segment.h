//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: segment.h 5588 2012-04-28 12:14:42Z wschweer $
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

class Measure;
class Segment;
class ChordRest;
class Lyrics;
class QPainter;
class Spanner;
class System;

//------------------------------------------------------------------------
//   @@ Segment
///   A segment holds all vertical aligned staff elements.
///   Segments are typed and contain only Elements of the same type.
//
//    @P subtype SegmentType
//------------------------------------------------------------------------

/**
 All Elements in a segment start at the same tick. The Segment can store one Element for
 each voice in each staff in the score. It also stores the lyrics for each staff.
 Some elements (Clef, KeySig, TimeSig etc.) are assumed to always have voice zero
 and can be found in _elist[staffIdx * VOICES];

 Segments are children of Measures and store Clefs, KeySigs, TimeSigs,
 BarLines and ChordRests.

 special case SegGrace:
      - tick()                 is play position of acciaccatura and appoggiatura notes
      - tick() - tickOffset()  is logical position, relevant for layout
                               (logicl position is the tick position of the next main note)
*/

class Segment : public Element {
      Q_OBJECT
      Q_PROPERTY(SegmentType subtype READ subtype WRITE setSubtype)
      Q_ENUMS(SegmentType)

   public:
      enum SegmentType {
            SegClef               = 0x1,
            SegKeySig             = 0x2,
            SegTimeSig            = 0x4,
            SegStartRepeatBarLine = 0x8,
            SegBarLine            = 0x10,
            SegGrace              = 0x20,
            SegChordRest          = 0x40,
            SegBreath             = 0x80,
            SegEndBarLine         = 0x100,
            SegTimeSigAnnounce    = 0x200,
            SegKeySigAnnounce     = 0x400,
            SegAll                = 0xfff,
            SegChordRestGrace     = SegChordRest | SegGrace
            };
      typedef QFlags<SegmentType> SegmentTypes;

   private:
      Segment* _next;               // linked list of segments inside a measure
      Segment* _prev;

      mutable bool empty;           // cached value
      mutable bool _written;        // used for write()

      SegmentType _subtype;
      int _tick;
      Spatium _extraLeadingSpace;
      Spatium _extraTrailingSpace;
      QList<qreal>   _dotPosX;     ///< size = staves

      Spanner* _spannerFor;
      Spanner* _spannerBack;

      QList<Element*> _annotations;

      QList<Element*> _elist;      ///< Element storage, size = staves * VOICES.

      void init();
      void checkEmpty() const;
      void addSpanner(Spanner*);
      void removeSpanner(Spanner*);

   public:
      Segment(Measure* m = 0);
      Segment(Measure*, SegmentType, int tick);
      Segment(const Segment&);
      ~Segment();

      virtual Segment* clone() const    { return new Segment(*this); }
      virtual ElementType type() const  { return SEGMENT; }

      virtual void setScore(Score*);

      Q_INVOKABLE Segment* next() const             { return _next;   }
      Segment* next(SegmentTypes) const;

      void setNext(Segment* e)          { _next = e;      }
      Q_INVOKABLE Segment* prev() const { return _prev;   }
      Segment* prev(SegmentTypes) const;
      void setPrev(Segment* e)          { _prev = e;      }

      Q_INVOKABLE Segment* next1() const;
      Segment* next1(SegmentTypes) const;
      Q_INVOKABLE Segment* prev1() const;
      Segment* prev1(SegmentTypes) const;

      Segment* nextCR(int track = -1) const;

      ChordRest* nextChordRest(int track, bool backwards = false) const;

      Q_INVOKABLE Element* element(int track) const    { return _elist.value(track);  }
      const QList<Element*>& elist() const { return _elist; }

      void removeElement(int track);
      void setElement(int track, Element* el);
      const QList<Lyrics*>* lyricsList(int staffIdx) const;

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
      static SegmentType segmentType(ElementType type);
      SegmentType subtype() const                { return _subtype; }
      void setSubtype(SegmentType t)             { _subtype = t; }

      void removeGeneratedElements();
      bool isEmpty() const                       { return empty; }
      void fixStaffIdx();
      bool isChordRest() const                   { return _subtype == SegChordRest; }
      bool isGrace() const                       { return _subtype == SegGrace; }
      void setTick(int);
      int tick() const;
      int rtick() const                          { return _tick; } // tickposition relative to measure start
      void setRtick(int val)                     { _tick = val; }

      bool splitsTuplet() const;

      Spanner* spannerFor() const                { return _spannerFor;         }
      Spanner* spannerBack() const               { return _spannerBack;        }
      void addSpannerBack(Spanner* e);
      bool removeSpannerBack(Spanner* e);
      void addSpannerFor(Spanner* e);
      bool removeSpannerFor(Spanner* e);

      const QList<Element*>& annotations() const { return _annotations;        }
      void removeAnnotation(Element* e)          { _annotations.removeOne(e);  }

      qreal dotPosX(int staffIdx) const          { return _dotPosX[staffIdx];  }
      void setDotPosX(int staffIdx, qreal val)   { _dotPosX[staffIdx] = val;   }

      Spatium extraLeadingSpace() const          { return _extraLeadingSpace;  }
      void setExtraLeadingSpace(Spatium v)       { _extraLeadingSpace = v;     }
      Spatium extraTrailingSpace() const         { return _extraTrailingSpace; }
      void setExtraTrailingSpace(Spatium v)      { _extraTrailingSpace = v;    }
      bool written() const                       { return _written;            }
      void setWritten(bool val)                  { _written = val;             }
      virtual void write(Xml&) const;
      virtual void read(const QDomElement&);

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);

      bool operator<(const Segment&) const;
      bool operator>(const Segment&) const;
      };

Q_DECLARE_OPERATORS_FOR_FLAGS(Segment::SegmentTypes)
Q_DECLARE_METATYPE(Segment::SegmentType)

#endif

