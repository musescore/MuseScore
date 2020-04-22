//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __PIANOLEVELSFILTER_H__
#define __PIANOLEVELSFILTER_H__

namespace Ms {

class Note;
class NoteEvent;
class Staff;


//---------------------------------------------------------
//   PianoLevelsFilter
//       Manage note/event data for different views when drawing in the PianoLevels window
//---------------------------------------------------------

class PianoLevelsFilter {
public:
      static PianoLevelsFilter* FILTER_LIST[];

      virtual QString name() = 0;
      virtual int maxRange() = 0;
      virtual int minRange() = 0;
      virtual int divisionGap() = 0;  //Vertical guide line separation gap
      virtual bool isPerEvent() = 0;
      virtual int value(Staff* staff, Note* note, NoteEvent* evt) = 0;
      virtual void setValue(Staff* staff, Note* note, NoteEvent* evt, int value) = 0;
      };


//---------------------------------------------------------
//   PianoLevelFilterOnTime
//---------------------------------------------------------

class PianoLevelFilterOnTime : public PianoLevelsFilter {
      Q_DECLARE_TR_FUNCTIONS(PianoLevelFilterOnTime)

public:
      QString name() override { return tr("Note on time", "amount note 'on time' is adjusted by"); }
      int maxRange() override { return 1000; }
      int minRange() override { return -1000; }
      int divisionGap() override { return 250; }
      bool isPerEvent() override { return true; }
      int value(Staff* staff, Note* note, NoteEvent* evt) override;
      void setValue(Staff* staff, Note* note, NoteEvent* evt, int value) override;
      };


//---------------------------------------------------------
//   PianoLevelFilterLen
//---------------------------------------------------------


class PianoLevelFilterLen : public PianoLevelsFilter {
      Q_DECLARE_TR_FUNCTIONS(PianoLevelFilterLen)

public:
      QString name() override { return tr("Length as note multiplier", "length tweak is interpreted as a scalar to base note length"); }
      int maxRange() override { return 1000; }
      int minRange() override { return 0; }
      int divisionGap() override { return 250; }
      bool isPerEvent() override { return true; }
      int value(Staff* staff, Note* note, NoteEvent* evt) override;
      void setValue(Staff* staff, Note* note, NoteEvent* evt, int value) override;
      };


//---------------------------------------------------------
//   PianoLevelFilterLenOff
//---------------------------------------------------------


class PianoLevelFilterLenOfftime : public PianoLevelsFilter {
      Q_DECLARE_TR_FUNCTIONS(PianoLevelFilterLenOfftime)

public:
      QString name() override { return tr("Length as note off time", "length tweak is interpreted as an offset to the base note length"); }
      int maxRange() override;
      int minRange() override { return 0; }
      int divisionGap() override;
      bool isPerEvent() override { return true; }
      int value(Staff* staff, Note* note, NoteEvent* evt) override;
      void setValue(Staff* staff, Note* note, NoteEvent* evt, int value) override;
      };


//---------------------------------------------------------
//   PianoLevelFilterVeloOffset
//---------------------------------------------------------


class PianoLevelFilterVeloOffset : public PianoLevelsFilter {
      Q_DECLARE_TR_FUNCTIONS(PianoLevelFilterVeloOffset)

public:
      QString name() override { return tr("Velocity as dynamics multiplier", "velocity tweak is interpreted as a scalar relative to the dynamics marking"); }
      int maxRange() override { return 200; }
      int minRange() override { return -200; }
      int divisionGap() override { return 100; }
      bool isPerEvent() override { return false; }
      int value(Staff* staff, Note* note, NoteEvent* evt) override;
      void setValue(Staff* staff, Note* note, NoteEvent* evt, int value) override;
      };


//---------------------------------------------------------
//   PianoLevelFilterVeloUser
//---------------------------------------------------------


class PianoLevelFilterVeloUser : public PianoLevelsFilter {
      Q_DECLARE_TR_FUNCTIONS(PianoLevelFilterVeloUser)

public:
      QString name() override { return tr("Velocity as absolute MIDI volume", "velocity tweak is intepreted as a MIDI volume level"); }
      int maxRange() override { return 128; }
      int minRange() override { return 0; }
      int divisionGap() override { return 32; }
      bool isPerEvent() override { return false; }
      int value(Staff* staff, Note* note, NoteEvent* evt) override;
      void setValue(Staff* staff, Note* note, NoteEvent* evt, int value) override;
      };

}

#endif // __PIANOLEVELSFILTER_H__
