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

#ifndef __STAFF_H__
#define __STAFF_H__

/**
 \file
 Definition of class Staff.
*/

#include "mscore.h"
#include "key.h"
#include "velo.h"
#include "pitch.h"
#include "cleflist.h"
#include "stafftype.h"
#include "groups.h"

namespace Ms {

class Instrument;
class InstrumentTemplate;
class Xml;
class Part;
class Score;
class KeyList;
class StaffType;
class StaffTypeTablature;
class Staff;
class StringData;
class ClefList;
struct ClefTypeList;
class Segment;
class Clef;
class TimeSig;
class Ottava;

//---------------------------------------------------------
//   LinkedStaves
//---------------------------------------------------------

class LinkedStaves {
      QList<Staff*> _staves;

   public:
      LinkedStaves() {}
      QList<Staff*>& staves()             { return _staves; }
      const QList<Staff*>& staves() const { return _staves; }
      void add(Staff*);
      void remove(Staff*);
      bool isEmpty() const { return _staves.isEmpty(); }
      };

//---------------------------------------------------------
//   BracketItem
//---------------------------------------------------------

struct BracketItem {
      BracketType _bracket;
      int _bracketSpan;

      BracketItem() {
            _bracket = NO_BRACKET;
            _bracketSpan = 0;
            }
      BracketItem(BracketType a, int b) {
            _bracket = a;
            _bracketSpan = b;
            }
      };

//---------------------------------------------------------
//   @@ Staff
///   Global staff data not directly related to drawing.
//---------------------------------------------------------

class Staff : public QObject {
      Q_OBJECT

      Score* _score;
      Part* _part;
      int _rstaff;                  ///< Index in Part.

      ClefList clefs;
      std::map<int,TimeSig*> timesigs;
      KeyList _keymap;

      QList <BracketItem> _brackets;
      int _barLineSpan;       ///< 0 - no bar line, 1 - span this staff, ...
      int _barLineFrom;       ///< line of start staff to draw the barline from (0 = staff top line, ...)
      int _barLineTo;         ///< line of end staff to draw the bar line to (0= staff top line, ...)
      bool _small;
      bool _invisible;
      bool _updateKeymap;

      qreal _userDist;        ///< user edited extra distance

      StaffType* _staffType;

      LinkedStaves* _linkedStaves;

      QMap<int,int> _channelList[VOICES];

      VeloList _velocities;         ///< cached value
      PitchList _pitchOffsets;      ///< cached value

   public:
      Staff(Score* = 0);
      Staff(Score*, Part*, int);
      ~Staff();
      void init(const InstrumentTemplate*, const StaffType *staffType, int);
      void initFromStaffType(const StaffType* staffType);

      bool isTop() const             { return _rstaff == 0; }
      QString partName() const;
      int rstaff() const             { return _rstaff; }
      int idx() const;
      void setRstaff(int n)          { _rstaff = n;    }
      void read(XmlReader&);
      void read114(XmlReader&);
      void write(Xml& xml) const;
      Part* part() const             { return _part;        }
      void setPart(Part* p)          { _part = p;           }

      BracketType bracket(int idx) const;
      int bracketSpan(int idx) const;
      void setBracket(int idx, BracketType val);
      void setBracketSpan(int idx, int val);
      int bracketLevels() const      { return _brackets.size(); }
      void addBracket(BracketItem);
      QList <BracketItem> brackets() const { return _brackets; }
      void cleanupBrackets();

      ClefTypeList clefTypeList(int tick) const;
      ClefType clef(int tick) const;
      ClefType clef(Segment*) const;
      void addClef(Clef*);
      void removeClef(Clef*);
      void setClef(int, const ClefTypeList&);
      void setClef(int, ClefType);

      void addTimeSig(TimeSig*);
      void removeTimeSig(TimeSig*);
      Fraction timeStretch(int tick) const;
      TimeSig* timeSig(int tick) const;
      const Groups& group(int tick) const;

      KeyList* keymap()                   { return &_keymap;      }
      KeySigEvent key(int tick) const;
      int nextKeyTick(int tick) const;
      void setKey(int tick, int st);
      void setKey(int tick, const KeySigEvent& st);
      void removeKey(int tick);

      bool show() const;
      bool slashStyle() const;
      bool small() const             { return _small;       }
      void setSmall(bool val)        { _small = val;        }
      bool invisible() const         { return _invisible;   }
      void setInvisible(bool val)    { _invisible = val;    }
      void setSlashStyle(bool val);
      int lines() const;
      void setLines(int);
      qreal lineDistance() const;
      int barLineSpan() const        { return _barLineSpan; }
      int barLineFrom() const        { return _barLineFrom; }
      int barLineTo() const          { return _barLineTo;   }
      void setBarLineSpan(int val)   { _barLineSpan = val;  }
      void setBarLineFrom(int val)   { _barLineFrom = val;  }
      void setBarLineTo(int val)     { _barLineTo   = val;  }
      Score* score() const           { return _score;       }
      qreal mag() const;
      qreal height() const;
      qreal spatium() const;
      int channel(int tick, int voice) const;
      QMap<int,int>* channelList(int voice) { return  &_channelList[voice]; }

      StaffType* staffType() const     { return _staffType;      }
      void setStaffType(StaffType* st);
      StaffGroup staffGroup() const    { return _staffType->group(); }
      bool isPitchedStaff() const      { return staffGroup() == STANDARD_STAFF_GROUP; }
      bool isTabStaff() const          { return staffGroup() == TAB_STAFF_GROUP; }
      bool isDrumStaff() const         { return staffGroup() == PERCUSSION_STAFF_GROUP; }

      bool updateKeymap() const        { return _updateKeymap;   }
      void setUpdateKeymap(bool v)     { _updateKeymap = v;      }
      VeloList& velocities()           { return _velocities;     }
      PitchList& pitchOffsets()        { return _pitchOffsets;   }
      int pitchOffset(int tick)        { return _pitchOffsets.pitchOffset(tick);   }
      void updateOttava(Ottava*);

      LinkedStaves* linkedStaves() const    { return _linkedStaves; }
      void setLinkedStaves(LinkedStaves* l) { _linkedStaves = l;    }
      void linkTo(Staff* staff);
      bool primaryStaff() const;
      qreal userDist() const        { return _userDist;  }
      void setUserDist(qreal val)   { _userDist = val;  }
      void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      bool genKeySig();
      bool showLedgerLines();
      const ClefList& clefList() const { return clefs; }
      };

}     // namespace Ms
#endif

