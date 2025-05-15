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

#include "changeMap.h"
#include "cleflist.h"
#include "groups.h"
#include "keylist.h"
#include "mscore.h"
#include "pitch.h"
#include "scoreElement.h"
#include "stafftypelist.h"

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
class Note;

enum class Key;

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
      bool _mergeMatchingRests { false };       // merge matching rests in multiple voices
      HideMode _hideWhenEmpty  { HideMode::AUTO };    // hide empty staves

      QColor _color            { MScore::defaultColor };
      qreal _userDist          { 0.0   };       ///< user edited extra distance

      StaffTypeList _staffTypeList;

      QMap<int,int> _channelList[VOICES];
      QMap<int,SwingParameters> _swingList;
      QMap<int,int> _capoList;
      bool _playbackVoice[VOICES] { true, true, true, true };

      ChangeMap _velocities;         ///< cached value
      ChangeMap _velocityMultiplications;         ///< cached value
      PitchList _pitchOffsets;      ///< cached value

      void fillBrackets(int);
      void cleanBrackets();

      qreal mag(const StaffType*) const;

   public:
      Staff(Score* score = 0);
      void init(const InstrumentTemplate*, const StaffType *staffType, int);
      void initFromStaffType(const StaffType* staffType);
      void init(const Staff*);

      ElementType type() const override { return ElementType::STAFF; }

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
      void changeBracketColumn(int oldColumn, int newColumn);
      void addBracket(BracketItem*);
      const QList<BracketItem*>& brackets() const { return _brackets; }
      QList<BracketItem*>& brackets()             { return _brackets; }
      void cleanupBrackets();
      int bracketLevels() const;

      ClefList& clefList()                           { return clefs;  }
      ClefTypeList clefType(const Fraction&) const;
      ClefTypeList defaultClefType() const           { return _defaultClefType; }
      void setDefaultClefType(const ClefTypeList& l) { _defaultClefType = l; }
      ClefType clef(const Fraction&) const;
      Fraction nextClefTick(const Fraction&) const;
      Fraction currentClefTick(const Fraction&) const;

      void setClef(Clef*);
      void removeClef(const Clef*);

      void addTimeSig(TimeSig*);
      void removeTimeSig(TimeSig*);
      void clearTimeSig();
      Fraction timeStretch(const Fraction&) const;
      TimeSig* timeSig(const Fraction&) const;
      TimeSig* nextTimeSig(const Fraction&) const;
      Fraction currentTimeSigTick(const Fraction&) const;

      bool isLocalTimeSignature(const Fraction& tick) { return timeStretch(tick) != Fraction(1, 1); }

      const Groups& group(const Fraction&) const;

      KeyList* keyList()                      { return &_keys;                  }
      Key key(const Fraction& tick) const     { return keySigEvent(tick).key(); }
      KeySigEvent keySigEvent(const Fraction&) const;
      Fraction nextKeyTick(const Fraction&) const;
      Fraction currentKeyTick(const Fraction&) const;
      KeySigEvent prevKey(const Fraction&) const;
      void setKey(const Fraction&, KeySigEvent);
      void removeKey(const Fraction&);

      bool show() const;
      bool stemless(const Fraction&) const;
      bool cutaway() const           { return _cutaway;     }
      void setCutaway(bool val)      { _cutaway = val;      }
      bool showIfEmpty() const       { return _showIfEmpty; }
      void setShowIfEmpty(bool val)  { _showIfEmpty = val;  }

      bool hideSystemBarLine() const      { return _hideSystemBarLine; }
      void setHideSystemBarLine(bool val) { _hideSystemBarLine = val;  }
      HideMode hideWhenEmpty() const      { return _hideWhenEmpty;     }
      void setHideWhenEmpty(HideMode v)   { _hideWhenEmpty = v;        }
      bool mergeMatchingRests() const     { return _mergeMatchingRests;}
      void setMergeMatchingRests(bool val){ _mergeMatchingRests = val; }

      int barLineSpan() const        { return _barLineSpan; }
      int barLineFrom() const        { return _barLineFrom; }
      int barLineTo() const          { return _barLineTo;   }
      void setBarLineSpan(int val)   { _barLineSpan = val;  }
      void setBarLineFrom(int val)   { _barLineFrom = val;  }
      void setBarLineTo(int val)     { _barLineTo = val;    }
      qreal height() const;

      int channel(const Fraction&, int voice) const;

      QList<Note*> getNotes() const;
      void addChord(QList<Note*>& list, Chord* chord, int voice) const;

      void clearChannelList(int voice)                               { _channelList[voice].clear(); }
      void insertIntoChannelList(int voice, const Fraction& tick, int channelId) { _channelList[voice].insert(tick.ticks(), channelId); }

      SwingParameters swing(const Fraction&)  const;
      void clearSwingList()                                  { _swingList.clear(); }
      void insertIntoSwingList(const Fraction& tick, SwingParameters sp) { _swingList.insert(tick.ticks(), sp); }

      int capo(const Fraction&) const;
      void clearCapoList()                             { _capoList.clear(); }
      void insertIntoCapoList(const Fraction& tick, int fretId)    { _capoList.insert(tick.ticks(), fretId); }

      //==== staff type helper function
      const StaffType* staffType(const Fraction&) const;
      const StaffType* constStaffType(const Fraction&) const;
      const StaffType* staffTypeForElement(const Element*) const;
      StaffType* staffType(const Fraction&);
      StaffType* setStaffType(const Fraction&, const StaffType&);
      void removeStaffType(const Fraction&);
      void staffTypeListChanged(const Fraction&);

      bool isPitchedStaff(const Fraction&) const;
      bool isTabStaff(const Fraction&) const;
      bool isDrumStaff(const Fraction&) const;

      int lines(const Fraction&) const;
      void setLines(const Fraction&, int lines);
      qreal lineDistance(const Fraction&) const;

      bool invisible(const Fraction&) const;
      void setInvisible(const Fraction&, bool val);

      void setSlashStyle(const Fraction&, bool val);
      int middleLine(const Fraction&) const;
      int bottomLine(const Fraction&) const;

      qreal mag(const Fraction&) const;
      qreal mag(const Element*) const;
      qreal spatium(const Fraction&) const;
      qreal spatium(const Element*) const;
      //===========

      ChangeMap& velocities()           { return _velocities;     }
      ChangeMap& velocityMultiplications()      { return _velocityMultiplications;     }
      PitchList& pitchOffsets()        { return _pitchOffsets;   }

      int pitchOffset(const Fraction& tick) const { return _pitchOffsets.pitchOffset(tick.ticks());   }
      void updateOttava();

      QList<Staff*> staffList() const;
      bool primaryStaff() const;

      qreal userDist() const        { return _userDist;  }
      void setUserDist(qreal val)   { _userDist = val;   }

      void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      void localSpatiumChanged(double oldVal, double newVal, Fraction tick);
      bool genKeySig();
      bool showLedgerLines(const Fraction&) const;

      QColor color(const Fraction&) const;
      void setColor(const Fraction&, const QColor& val);
      void undoSetColor(const QColor& val);
      void insertTime(const Fraction&, const Fraction& len);

      QVariant getProperty(Pid) const override;
      bool setProperty(Pid, const QVariant&) override;
      QVariant propertyDefault(Pid) const override;

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

      void triggerLayout();
      void triggerLayout(const Fraction& tick);
      };

}     // namespace Ms
#endif

