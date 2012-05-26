//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: globals.h 5576 2012-04-24 19:15:22Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

extern bool enableExperimental;
extern bool scriptDebug;
extern bool noSeq;            ///< Dont use sequencer; cmd line option.
extern bool noMidi;           ///< Dont use midi; cmd line option.
extern bool midiInputTrace;   ///< debug option: dump midi input
extern bool midiOutputTrace;  ///< debug option: dump midi output
extern bool noGui;
extern bool converterMode;
extern double converterDpi;

//---------------------------------------------------------
//    ScoreState
//    used also to mask out shortcuts (actions.cpp)
//---------------------------------------------------------

enum ScoreState {
      STATE_INIT        = 0,
      STATE_DISABLED    = 1,
      STATE_NORMAL      = 2,
      STATE_NOTE_ENTRY  = 4,
      STATE_EDIT        = 8,
      STATE_LYRICS_EDIT = 16,
      STATE_PLAY        = 32,
      STATE_SEARCH      = 64,
      STATE_FOTO        = 128,
      STATE_ALL         = -1
      };

//---------------------------------------------------------
//   MidiRemoteType
//---------------------------------------------------------

enum MidiRemoteType {
      MIDI_REMOTE_TYPE_INACTIVE = -1,
      MIDI_REMOTE_TYPE_NOTEON = 0, MIDI_REMOTE_TYPE_CTRL
      };

//---------------------------------------------------------
//   MidiRemote
//---------------------------------------------------------

struct MidiRemote {
      int channel;
      MidiRemoteType type;
      int data;         // pitch or controller number
      };

extern const char* stateName(ScoreState);

static const qreal DPMM_DISPLAY = 4;   // 100 DPI
static const qreal PALETTE_SPATIUM = 1.9 * DPMM_DISPLAY;

extern QPaintDevice* pdev;
#endif
