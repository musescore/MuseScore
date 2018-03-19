//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: keyb.cpp 5658 2012-05-21 18:40:58Z wschweer $
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

#include "scoreview.h"
#include "libmscore/note.h"
#include "libmscore/sym.h"
#include "libmscore/note.h"
#include "libmscore/score.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/select.h"
#include "libmscore/input.h"
#include "libmscore/key.h"
#include "libmscore/measure.h"
#include "musescore.h"
#include "libmscore/slur.h"
#include "libmscore/tuplet.h"
#include "libmscore/text.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "drumtools.h"
#include "preferences.h"
#include "libmscore/segment.h"
#include "libmscore/mscore.h"
#include "libmscore/stafftype.h"
#include "texttools.h"

namespace Ms {

//---------------------------------------------------------
//   Canvas::editCmd
//---------------------------------------------------------

void ScoreView::editCmd(const QString& cmd)
      {
      if (!editData.element)
            return;

      if (editData.element->isLyrics()) {
            if (cmd == "next-lyric")
                  lyricsTab(false, true, false);
            else if (cmd == "prev-lyric")
                  lyricsTab(true, true, false);
            }
      }

//---------------------------------------------------------
//   updateInputState
//---------------------------------------------------------

void MuseScore::updateInputState(Score* score)
      {
      InputState& is = score->inputState();
      if (is.noteEntryMode()) {
            if (is.usingNoteEntryMethod(NoteEntryMethod::REPITCH))
                  is.setDuration(is.cr()->durationType());
            Staff* staff = score->staff(is.track() / VOICES);
            switch (staff->staffType(is.tick())->group()) {
                  case StaffGroup::STANDARD:
                        changeState(STATE_NOTE_ENTRY_STAFF_PITCHED);
                        break;
                  case StaffGroup::TAB:
                        changeState(STATE_NOTE_ENTRY_STAFF_TAB);
                        break;
                  case StaffGroup::PERCUSSION:
                        changeState(STATE_NOTE_ENTRY_STAFF_DRUM);
                        break;
                  }
            }

      getAction("pad-rest")->setChecked(is.rest());
      getAction("pad-dot")->setChecked(is.duration().dots() == 1);
      getAction("pad-dotdot")->setChecked(is.duration().dots() == 2);
      getAction("pad-dot3")->setChecked(is.duration().dots() == 3);
      getAction("pad-dot4")->setChecked(is.duration().dots() == 4);
      if ((mscore->state() & STATE_NORMAL) | (mscore->state() & STATE_NOTE_ENTRY)) {
            getAction("pad-dot")->setEnabled(true);
            getAction("pad-dotdot")->setEnabled(true);
            getAction("pad-dot3")->setEnabled(true);
            getAction("pad-dot4")->setEnabled(true);
            }
      switch (is.duration().type()) {
            case TDuration::DurationType::V_1024TH:
                  getAction("pad-dot")->setChecked(false);
                  getAction("pad-dot")->setEnabled(false);
                  // fall through
            case TDuration::DurationType::V_512TH:
                  getAction("pad-dotdot")->setChecked(false);
                  getAction("pad-dotdot")->setEnabled(false);
                  // fall through
            case TDuration::DurationType::V_256TH:
                  getAction("pad-dot3")->setChecked(false);
                  getAction("pad-dot3")->setEnabled(false);
                  // fall through
            case TDuration::DurationType::V_128TH:
                  getAction("pad-dot4")->setChecked(false);
                  getAction("pad-dot4")->setEnabled(false);
            default:
                  break;
            }

      getAction("note-longa")->setChecked(is.duration()  == TDuration::DurationType::V_LONG);
      getAction("note-breve")->setChecked(is.duration()  == TDuration::DurationType::V_BREVE);
      getAction("pad-note-1")->setChecked(is.duration()  == TDuration::DurationType::V_WHOLE);
      getAction("pad-note-2")->setChecked(is.duration()  == TDuration::DurationType::V_HALF);
      getAction("pad-note-4")->setChecked(is.duration()  == TDuration::DurationType::V_QUARTER);
      getAction("pad-note-8")->setChecked(is.duration()  == TDuration::DurationType::V_EIGHTH);
      getAction("pad-note-16")->setChecked(is.duration() == TDuration::DurationType::V_16TH);
      getAction("pad-note-32")->setChecked(is.duration() == TDuration::DurationType::V_32ND);
      getAction("pad-note-64")->setChecked(is.duration() == TDuration::DurationType::V_64TH);
      getAction("pad-note-128")->setChecked(is.duration() == TDuration::DurationType::V_128TH);

      // uncheck all voices if multi-selection
      int voice = score->selection().isSingle() ? is.voice() : -1;
      getAction("voice-1")->setChecked(voice == 0);
      getAction("voice-2")->setChecked(voice == 1);
      getAction("voice-3")->setChecked(voice == 2);
      getAction("voice-4")->setChecked(voice == 3);

      getAction("acciaccatura")->setChecked(is.noteType() == NoteType::ACCIACCATURA);
      getAction("appoggiatura")->setChecked(is.noteType() == NoteType::APPOGGIATURA);
      getAction("grace4")->setChecked(is.noteType()  == NoteType::GRACE4);
      getAction("grace16")->setChecked(is.noteType() == NoteType::GRACE16);
      getAction("grace32")->setChecked(is.noteType() == NoteType::GRACE32);
      getAction("grace8after")->setChecked(is.noteType()  == NoteType::GRACE8_AFTER);
      getAction("grace16after")->setChecked(is.noteType() == NoteType::GRACE16_AFTER);
      getAction("grace32after")->setChecked(is.noteType() == NoteType::GRACE32_AFTER);
      getAction("beam-start")->setChecked(is.beamMode() == Beam::Mode::BEGIN);
      getAction("beam-mid")->setChecked(is.beamMode()   == Beam::Mode::MID);
      getAction("no-beam")->setChecked(is.beamMode()    == Beam::Mode::NONE);
      getAction("beam32")->setChecked(is.beamMode()     == Beam::Mode::BEGIN32);
      getAction("auto-beam")->setChecked(is.beamMode()  == Beam::Mode::AUTO);

      if(is.noteEntryMode() && !is.rest())
            updateShadowNote();
      }
}

