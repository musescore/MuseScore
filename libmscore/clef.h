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

namespace Ms {

class XmlWriter;
class MuseScoreView;
class Segment;

static const int NO_CLEF = -1000;


//---------------------------------------------------------
//   ClefType
//---------------------------------------------------------

enum class ClefType : signed char {
      INVALID = -1,
      G = 0,
      G15_MB,
      G8_VB,
      G8_VA,
      G15_MA,
      G8_VB_O,
      G8_VB_P,
      G_1,
      C1,
      C2,
      C3,
      C4,
      C5,
      C_19C,
      C3_F18C,
      C4_F18C,
      C3_F20C,
      C4_F20C,
      F,
      F15_MB,
      F8_VB,
      F_8VA,
      F_15MA,
      F_B,
      F_C,
      F_F18C,
      F_19C,
      PERC,
      PERC2,
      TAB,
      TAB4,
      TAB_SERIF,
      TAB4_SERIF,
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
      int _line;               ///< Line for musicXml and for positioning on the staff
      int _octChng;            ///< Octave change for musicXml.
      int _pitchOffset;        ///< Pitch offset for line 0.
      signed char _lines[14];
      SymId _symId;
      const char* _name;
      StaffGroup _staffGroup;

   public:
      static const char* tag(ClefType t)       { return clefTable[int(t)]._tag;         }
      static const char* sign(ClefType t)      { return clefTable[int(t)]._sign;        }
      static int line(ClefType t)              { return clefTable[int(t)]._line;        }
      static int octChng(ClefType t)           { return clefTable[int(t)]._octChng;     }
      static int pitchOffset(ClefType t)       { return clefTable[int(t)]._pitchOffset; }
      static SymId symId(ClefType t)           { return clefTable[int(t)]._symId;       }
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

class Clef final : public Element {
      SymId symId;
      bool _showCourtesy;
      bool _small;

      ClefTypeList _clefTypes;

   public:
      Clef(Score*);
      Clef(const Clef&);
      ~Clef() {}
      virtual Clef* clone() const        { return new Clef(*this); }
      virtual ElementType type() const { return ElementType::CLEF; }
      virtual qreal mag() const;

      Segment* segment() const           { return (Segment*)parent(); }
      Measure* measure() const           { return (Measure*)parent()->parent(); }

      virtual bool acceptDrop(EditData&) const override;
      virtual Element* drop(EditData&);
      virtual void layout();
      virtual void draw(QPainter*) const;
      virtual void read(XmlReader&);
      virtual void write(XmlWriter&) const;

      virtual bool isEditable() const                    { return false; }

      bool small() const               { return _small; }
      void setSmall(bool val);

      int tick() const;

      bool showCourtesy() const        { return _showCourtesy; }
      void setShowCourtesy(bool v)     { _showCourtesy = v; }
      void undoSetShowCourtesy(bool v);

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
      virtual void spatiumChanged(qreal oldValue, qreal newValue) override;

      QVariant getProperty(Pid propertyId) const;
      bool setProperty(Pid propertyId, const QVariant&);
      QVariant propertyDefault(Pid id) const;

      virtual Element* nextSegmentElement() override;
      virtual Element* prevSegmentElement() override;
      QString accessibleInfo() const override;
      void clear();
      };

}     // namespace Ms
#endif

