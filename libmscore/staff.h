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
#include "velo.h"
#include "pitch.h"
#include "cleflist.h"
#include "keylist.h"
#include "stafftypelist.h"
#include "groups.h"
#include "scoreElement.h"

namespace Ms {

class InstrumentTemplate;
class XmlWriter;
class Part;
class Score;
class KeyList;
class StaffType;
class Staff;
struct ClefTypeList;
class Segment;
class Clef;
class TimeSig;
class Ottava;
class BracketItem;

enum class Key;

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
      bool empty() const { return _staves.empty(); }
      };

//---------------------------------------------------------
//   SwingParameters
//---------------------------------------------------------

struct SwingParameters {
      int swingUnit;
      int swingRatio;
      };

//---------------------------------------------------------
//    Staff
///    Global staff data not directly related to drawing.
//---------------------------------------------------------

class Staff final : public ScoreElement {
   public:
      enum class HideMode { AUTO, ALWAYS, NEVER, INSTRUMENT };

   private:
      Part* _part       { 0 };

      ClefList clefs;
      ClefTypeList _defaultClefType;

      KeyList _keys;
      std::map<int,TimeSig*> timesigs;

      QList <BracketItem*> _brackets;
      int  _barLineSpan        { false };    ///< true - span barline to next staff
      int _barLineFrom         { 0     };    ///< line of start staff to draw the barline from (0 = staff top line, ...)
      int _barLineTo           { 0     };    ///< line of end staff to draw the bar line to (0= staff bottom line, ...)

      bool _invisible          { false };
      bool _cutaway            { false };
      bool _showIfEmpty        { false };       ///< show this staff if system is empty and hideEmptyStaves is true
      bool _hideSystemBarLine  { false };       // no system barline if not preceded by staff with barline
      HideMode _hideWhenEmpty  { HideMode::AUTO };    // hide empty staves

      QColor _color            { MScore::defaultColor };
      qreal _userDist          { 0.0   };       ///< user edited extra distance

      StaffTypeList _staffTypeList;

      LinkedStaves* _linkedStaves { 0 };
      QMap<int,int> _channelList[VOICES];
      QMap<int,SwingParameters> _swingList;
      bool _playbackVoice[VOICES] { true, true, true, true };

      VeloList _velocities;         ///< cached value
      PitchList _pitchOffsets;      ///< cached value

      void scaleChanged(double oldValue, double newValue);
      void fillBrackets(int);
      void cleanBrackets();

   public:
      Staff(Score* score = 0) : ScoreElement(score) {}
      ~Staff();
      void init(const InstrumentTemplate*, const StaffType *staffType, int);
      void initFromStaffType(const StaffType* staffType);
      void init(const Staff*);

      virtual ElementType type() const override { return ElementType::STAFF; }

      bool isTop() const;
      QString partName() const;
      int rstaff() const;
      int idx() const;
      void read(XmlReader&);
      bool readProperties(XmlReader&);
      void write(XmlWriter& xml) const;
      Part* part() const             { return _part;        }
      void setPart(Part* p)          { _part = p;           }

      BracketType bracketType(int idx) const;
      int bracketSpan(int idx) const;
      void setBracketType(int idx, BracketType val);
      void setBracketSpan(int idx, int val);
      void swapBracket(int oldIdx, int newIdx);
      void addBracket(BracketItem*);
      const QList<BracketItem*>& brackets() const { return _brackets; }
      QList<BracketItem*>& brackets()             { return _brackets; }
      void cleanupBrackets();
      int bracketLevels() const;

      ClefList& clefList()                           { return clefs;  }
      ClefTypeList clefType(int tick) const;
      ClefTypeList defaultClefType() const           { return _defaultClefType; }
      void setDefaultClefType(const ClefTypeList& l) { _defaultClefType = l; }
      ClefType clef(int tick) const;
      int nextClefTick(int tick) const;

      void setClef(Clef*);
      void removeClef(Clef*);

      void addTimeSig(TimeSig*);
      void removeTimeSig(TimeSig*);
      void clearTimeSig();
      Fraction timeStretch(int tick) const;
      TimeSig* timeSig(int tick) const;
      TimeSig* nextTimeSig(int tick) const;
      bool isLocalTimeSignature(int tick) { return timeStretch(tick) != Fraction(1, 1); }

      const Groups& group(int tick) const;

      KeyList* keyList()               { return &_keys;                  }
      Key key(int tick) const          { return keySigEvent(tick).key(); }
      KeySigEvent keySigEvent(int tick) const;
      int nextKeyTick(int tick) const;
      int currentKeyTick(int tick) const;
      KeySigEvent prevKey(int tick) const;
      void setKey(int tick, KeySigEvent);
      void removeKey(int tick);

      bool show() const;
      bool slashStyle(int tick) const;
      bool invisible() const         { return _invisible;   }
      void setInvisible(bool val)    { _invisible = val;    }
      bool cutaway() const           { return _cutaway;     }
      void setCutaway(bool val)      { _cutaway = val;      }
      bool showIfEmpty() const       { return _showIfEmpty; }
      void setShowIfEmpty(bool val)  { _showIfEmpty = val;  }

      bool hideSystemBarLine() const      { return _hideSystemBarLine; }
      void setHideSystemBarLine(bool val) { _hideSystemBarLine = val;  }
      HideMode hideWhenEmpty() const      { return _hideWhenEmpty;     }
      void setHideWhenEmpty(HideMode v)   { _hideWhenEmpty = v;        }

      int barLineSpan() const        { return _barLineSpan; }
      int barLineFrom() const        { return _barLineFrom; }
      int barLineTo() const          { return _barLineTo;   }
      void setBarLineSpan(int val)   { _barLineSpan = val;  }
      void setBarLineFrom(int val)   { _barLineFrom = val;  }
      void setBarLineTo(int val)     { _barLineTo = val;    }
      qreal height() const;
      int channel(int tick, int voice) const;
      QMap<int,int>* channelList(int voice) { return  &_channelList[voice]; }
      SwingParameters swing(int tick)  const;
      QMap<int,SwingParameters>* swingList() { return &_swingList; }

      //==== staff type
      const StaffType* staffType(int tick) const;
      StaffType* staffType(int tick);
      StaffType* setStaffType(int tick, const StaffType*);
      void staffTypeListChanged(int tick);

      bool isPitchedStaff(int tick) const;
      bool isTabStaff(int tick) const;
      bool isDrumStaff(int tick) const;

      int lines(int tick) const;
      void setLines(int tick, int lines);
      qreal lineDistance(int tick) const;

      void setSlashStyle(int tick, bool val);
      int middleLine(int tick) const;
      int bottomLine(int tick) const;

      qreal userMag(int tick) const;
      void setUserMag(int tick, qreal m);
      qreal mag(int tick) const;
      bool small(int tick) const;
      void setSmall(int tick, bool val);
      qreal spatium(int tick) const;
      //===========

      VeloList& velocities()           { return _velocities;     }
      PitchList& pitchOffsets()        { return _pitchOffsets;   }

      int pitchOffset(int tick)        { return _pitchOffsets.pitchOffset(tick);   }
      void updateOttava();

      LinkedStaves* linkedStaves() const    { return _linkedStaves; }
      void setLinkedStaves(LinkedStaves* l) { _linkedStaves = l;    }
      QList<Staff*> staffList() const;
      void linkTo(Staff* staff);
      bool isLinked(Staff* staff);
      void unlink(Staff* staff);
      bool primaryStaff() const;

      qreal userDist() const        { return _userDist;  }
      void setUserDist(qreal val)   { _userDist = val;   }

      void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      bool genKeySig();
      bool showLedgerLines(int tick);

      QColor color() const                { return _color; }
      void setColor(const QColor& val)    { _color = val;    }
      void undoSetColor(const QColor& val);
      void insertTime(int tick, int len);

      virtual QVariant getProperty(P_ID) const override;
      virtual bool setProperty(P_ID, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;

      BracketType innerBracket() const;

      bool playbackVoice(int voice) const        { return _playbackVoice[voice]; }
      void setPlaybackVoice(int voice, bool val) { _playbackVoice[voice] = val; }

#ifndef NDEBUG
      void dumpClefs(const char* title) const;
      void dumpKeys(const char* title) const;
      void dumpTimeSigs(const char*) const;
#else
      void dumpClefs(const char*) const {}
      void dumpKeys(const char*) const {}
      void dumpTimeSigs(const char*) const {}
#endif
      };

}     // namespace Ms
#endif

