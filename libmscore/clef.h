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

#ifndef __CLEF_H__
#define __CLEF_H__

/**
 \file
 Definition of classes Clef and ClefList.
*/

#include "element.h"
#include "mscore.h"

class QPainter;

namespace Ms {

class Xml;
class MuseScoreView;
class Segment;

static const int NO_CLEF = -1000;

//---------------------------------------------------------
//   ClefType
//---------------------------------------------------------

enum class ClefType : signed char {
      INVALID = -1,
      G = 0,
      G1,
      G2,
      G3,
      F,
      F8,
      F15,
      F_B,
      F_C,
      C1,
      C2,
      C3,
      C4,
      TAB,
      PERC,
      C5,
      G4,
      F_8VA,
      F_15MA,
      PERC2,
      TAB2,
      MAX
      };

//---------------------------------------------------------
//   ClefTypeList
//---------------------------------------------------------

struct ClefTypeList {
      ClefType _concertClef;
      ClefType _transposingClef;

      ClefTypeList() {}
      ClefTypeList(ClefType a, ClefType b) : _concertClef(a), _transposingClef(b) {}
      bool operator==(const ClefTypeList& t) const;
      bool operator!=(const ClefTypeList& t) const;
      };

//---------------------------------------------------------
//   ClefInfo
///   Info about a clef.
//---------------------------------------------------------

class ClefInfo {
   public:
      static const ClefInfo clefTable[];

      const char* _tag;        ///< comprehensive name for instruments.xml
      const char* _sign;       ///< Name for musicXml.
      int _line;               ///< Line for musicXml.
      int _octChng;            ///< Octave change for musicXml.
      int _pitchOffset;        ///< Pitch offset for line 0.
      char _lines[14];
      const char* _name;
      StaffGroup _staffGroup;

   public:
      static const char* tag(ClefType t)       { return clefTable[int(t)]._tag;         }
      static const char* sign(ClefType t)      { return clefTable[int(t)]._sign;        }
      static int line(ClefType t)              { return clefTable[int(t)]._line;        }
      static int octChng(ClefType t)           { return clefTable[int(t)]._octChng;     }
      static int pitchOffset(ClefType t)       { return clefTable[int(t)]._pitchOffset; }
      static const char* lines(ClefType t)     { return clefTable[int(t)]._lines;       }
      static const char* name(ClefType t)      { return clefTable[int(t)]._name;        }
      static StaffGroup staffGroup(ClefType t) { return clefTable[int(t)]._staffGroup;  }
      static ClefType tag2type(const QString&);
      };

//---------------------------------------------------------
//   @@ Clef
///   Graphic representation of a clef.
//
//    @P showCourtesy bool
//    @P small        bool      r/o, set by layout
//---------------------------------------------------------

class Clef : public Element {
      Q_OBJECT
      Q_PROPERTY(bool showCourtesy READ showCourtesy WRITE undoSetShowCourtesy)
      Q_PROPERTY(bool small READ small)

      QList<Element*> elements;
      bool _showCourtesy;
      bool _showPreviousClef;       // show clef type at position tick-1
                                    // used for first clef on staff immediatly followed
                                    // by a different clef at same tick position
      bool _small;

      ClefTypeList _clefTypes;

      ClefType    curClefType;      // cached value of clef type (for re-laying out)
      int         curLines;         // cached value of staff nm. of lines  ( " )
      qreal       curLineDist;      // cached value of staff line distance ( " )
      void        layout1();        // lays the element out, using cached values

   public:
      Clef(Score*);
      Clef(const Clef&);
      virtual Clef* clone() const      { return new Clef(*this); }
      virtual ElementType type() const { return CLEF; }
      virtual void setSelected(bool f);
      virtual qreal mag() const;

      Segment* segment() const         { return (Segment*)parent(); }
      Measure* measure() const         { return (Measure*)parent()->parent(); }

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual void draw(QPainter*) const;
      virtual void read(XmlReader&);
      virtual void write(Xml&) const;

      virtual void addElement(Element* e, qreal x, qreal y);
      virtual Space space() const      { return Space(0.0, bbox().x() * 2.0 + width()); }

      bool small() const               { return _small; }
      void setSmall(bool val);

      int tick() const;

      bool showCourtesy() const        { return _showCourtesy; }
      void setShowCourtesy(bool v)     { _showCourtesy = v; }
      void undoSetShowCourtesy(bool v);

      static ClefType clefType(const QString& s);

      ClefType clefType() const;
      void setClefType(ClefType i);
      void setClefType(const QString& s);

      ClefTypeList clefTypeList() const     { return _clefTypes;                  }
      ClefType concertClef() const          { return _clefTypes._concertClef;     }
      ClefType transposingClef() const      { return _clefTypes._transposingClef; }
      void setConcertClef(ClefType val);
      void setTransposingClef(ClefType val);
      void setClefType(const ClefTypeList& ctl) { _clefTypes = ctl; }
      virtual void spatiumChanged(qreal oldValue, qreal newValue);

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant&);
      QVariant propertyDefault(P_ID id) const;
      };


}     // namespace Ms
#endif

