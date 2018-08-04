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
 Definition of classes Clef
*/

#include "element.h"
#include "mscore.h"

class QPainter;

namespace Ms {

class Xml;
class MuseScoreView;
class Segment;
class Symbol;

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
      PERC2,            // no longer supported, but kept for compat. with old scores; rendered as PERC
      TAB2,
      G5,
      MAX
      };

//---------------------------------------------------------
//   ClefTypeList
//---------------------------------------------------------

struct ClefTypeList {
      ClefType _concertClef = ClefType::G;
      ClefType _transposingClef = ClefType::G;

      ClefTypeList() {}
      ClefTypeList(ClefType a, ClefType b) : _concertClef(a), _transposingClef(b) {}
      ClefTypeList(ClefType a) : _concertClef(a), _transposingClef(a) {}
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
      signed char _lines[14];
      const char* _name;
      StaffGroup _staffGroup;

   public:
      static const char* tag(ClefType t)       { return clefTable[int(t)]._tag;         }
      static const char* sign(ClefType t)      { return clefTable[int(t)]._sign;        }
      static int line(ClefType t)              { return clefTable[int(t)]._line;        }
      static int octChng(ClefType t)           { return clefTable[int(t)]._octChng;     }
      static int pitchOffset(ClefType t)       { return clefTable[int(t)]._pitchOffset; }
      static const signed char* lines(ClefType t)     { return clefTable[int(t)]._lines;       }
      static const char* name(ClefType t)      { return clefTable[int(t)]._name;        }
      static StaffGroup staffGroup(ClefType t) { return clefTable[int(t)]._staffGroup;  }
      static ClefType tag2type(const QString&);
      };

//---------------------------------------------------------
//   @@ Clef
///    Graphic representation of a clef.
//
//   @P showCourtesy  bool    show/hide courtesy clef when applicable
//   @P small         bool    small, mid-staff clef (read only, set by layout)
//---------------------------------------------------------

class Clef : public Element {
      Q_OBJECT
      Q_PROPERTY(bool showCourtesy READ showCourtesy WRITE undoSetShowCourtesy)
      Q_PROPERTY(bool small READ small)

      Symbol* symbol;
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
      ~Clef();
      virtual Clef* clone() const        { return new Clef(*this); }
      virtual Element::Type type() const { return Element::Type::CLEF; }
      virtual void setSelected(bool f);
      virtual qreal mag() const;

      Segment* segment() const           { return (Segment*)parent(); }
      Measure* measure() const           { return (Measure*)parent()->parent(); }

      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual void draw(QPainter*) const;
      virtual void read(XmlReader&);
      virtual void write(Xml&) const;

      virtual bool isEditable() const                    { return false; }

      virtual Space space() const      { return Space(0.0, bbox().x() * 2.0 + width()); }
      SymId sym() const;
      qreal symMag() const;

      bool small() const               { return _small; }
      void setSmall(bool val);

      int tick() const;

      bool showCourtesy() const        { return _showCourtesy; }
      void setShowCourtesy(bool v)     { _showCourtesy = v; }
      void undoSetShowCourtesy(bool v);
      Clef* otherClef();

      static ClefType clefType(const QString& s);
      const char* clefTypeName();

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

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;
      QString accessibleInfo() override;
      };

}     // namespace Ms
#endif

