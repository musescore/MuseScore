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

#ifdef Q_OS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#endif

//---------------------------------------------------------
//   Canvas::editCmd
//---------------------------------------------------------

void ScoreView::editCmd(const QString& cmd)
      {
      if (!editObject)
            return;

      if (editObject->type() == ElementType::LYRICS) {
            if (cmd == "next-lyric")
                  lyricsTab(false, true, false);
            else if (cmd == "prev-lyric")
                  lyricsTab(true, true, false);
            }
      }

//---------------------------------------------------------
//   editKeyLyrics
//---------------------------------------------------------

bool ScoreView::editKeyLyrics(QKeyEvent* ev)
      {
      int key                         = ev->key();
      Qt::KeyboardModifiers modifiers = ev->modifiers();
      QString s                       = ev->text();
      bool ctrl                       = modifiers == Qt::ControlModifier;

      switch(key) {
            case Qt::Key_Space:
                  if (!(modifiers & CONTROL_MODIFIER)) {
                        if (s == "_")
                              lyricsUnderscore();
                        else // TODO: shift+tab events are filtered by qt
                              lyricsTab(modifiers & Qt::ShiftModifier, true, false);
                        }
                  else
                        return false;
                  break;

            case Qt::Key_Left:
                  if (!ctrl && editObject->edit(this, curGrip, key, modifiers, s)) {
                        mscore->textTools()->updateTools();
                        _score->update();
                        mscore->endCmd();
                        }
                  else
                        lyricsTab(true, true, true);      // go to previous lyrics
                  break;
            case Qt::Key_Right:
                  if (!ctrl && editObject->edit(this, curGrip, key, modifiers, s)) {
                        mscore->textTools()->updateTools();
                        _score->update();
                        mscore->endCmd();
                        }
                  else
                        lyricsTab(false, false, true);    // go to next lyrics
                  break;
            case Qt::Key_Up:
                  lyricsUpDown(true, true);
                  break;
            case Qt::Key_Down:
                  lyricsUpDown(false, true);
                  break;
            case Qt::Key_Return:
                  lyricsReturn();
                  break;
            default:
                  {
                  if(s == "-" && !(modifiers & CONTROL_MODIFIER))
                        lyricsMinus();
                  else if (s == "_" && !(modifiers & CONTROL_MODIFIER))
                        lyricsUnderscore();
                  else
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   editKey
//---------------------------------------------------------

void ScoreView::editKey(QKeyEvent* ev)
      {
      if (ev->type() == QEvent::KeyRelease) {
            auto modifiers = Qt::ControlModifier | Qt::ShiftModifier;
            if (editObject && editObject->isText()
               && ((ev->modifiers() & modifiers) == 0)) {
                  Text* text = static_cast<Text*>(editObject);
                  text->endHexState();
                  ev->accept();
                  update();
                  }
            return;
            }
      int key                         = ev->key();
      Qt::KeyboardModifiers modifiers = ev->modifiers();
      QString s                       = ev->text();

      if (MScore::debugMode)
            qDebug("keyPressEvent key 0x%02x(%c) mod 0x%04x <%s> nativeKey 0x%02x scancode %d",
               key, key, int(modifiers), qPrintable(s), ev->nativeVirtualKey(), ev->nativeScanCode());

      if (!editObject)
            return;

      if (editObject->type() == ElementType::LYRICS) {
            if (editKeyLyrics(ev)) {
                  ev->accept();
                  return;
                  }
            }
      else if (editObject->type() == ElementType::HARMONY) {
/*
            if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
                  harmonyTab(key == Qt::Key_Backtab ? true : (modifiers & Qt::ShiftModifier));
                  ev->accept();
                  return;
                  }
*/
            if (key == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  harmonyBeatsTab(true, modifiers & Qt::ShiftModifier);
                  ev->accept();
                  return;
                  }
/*
            if (key == Qt::Key_Semicolon || key == Qt::Key_Colon) {
                  harmonyBeatsTab(false, key == Qt::Key_Colon);
                  ev->accept();
                  return;
                  }
            if (key >= Qt::Key_1 && key <= Qt::Key_9 && (modifiers & CONTROL_MODIFIER)) {
                  int ticks = (MScore::division >> 4) << (key - Qt::Key_1);
                  harmonyTicksTab(ticks);
                  ev->accept();
                  return;
                  }
*/
            }
      else if (editObject->type() == ElementType::FIGURED_BASS) {
            int found = false;
            if (key == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  figuredBassTab(false, modifiers & Qt::ShiftModifier);
                  found = true;
                  }
            /*
            if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
                  figuredBassTab(true, key == Qt::Key_Backtab ? true : (modifiers & Qt::ShiftModifier) );
                  found = true;
                  }
            if (key >= Qt::Key_1 && key <= Qt::Key_9 && (modifiers & CONTROL_MODIFIER)) {
                  int ticks = (MScore::division >> 4) << (key - Qt::Key_1);
                  figuredBassTicksTab(ticks);
                  found = true;
                  }
*/
            if (found) {
                  ev->accept();
                  return;
                  }
            }

      if (!((modifiers & Qt::ShiftModifier) && (key == Qt::Key_Backtab))) {
            if (editObject->edit(this, curGrip, key, modifiers, s)) {
                  if (editObject->isText())
                        mscore->textTools()->updateTools();
                  updateGrips();
                  ev->accept();
                  _score->update();
                  mscore->endCmd();
                  return;
                  }
            if (editObject->isText() && (key == Qt::Key_Left || key == Qt::Key_Right)) {
                  ev->accept();
                  _score->update();
                  mscore->endCmd();
                  //return;
                  }
            }
      QPointF delta;
      qreal _spatium = editObject->spatium();

      qreal xval, yval;
      if (editObject->type() == ElementType::BEAM) {
            xval = 0.25 * _spatium;
            if (modifiers & Qt::ControlModifier)
                  xval = _spatium;
            else if (modifiers & Qt::AltModifier)
                  xval = 4 * _spatium;
            }
      else {
            xval = MScore::nudgeStep * _spatium;
            if (modifiers & Qt::ControlModifier)
                  xval = MScore::nudgeStep10 * _spatium;
            else if (modifiers & Qt::AltModifier)
                  xval = MScore::nudgeStep50 * _spatium;
            }
      yval = xval;

      if (mscore->vRaster()) {
            qreal vRaster = _spatium / MScore::vRaster();
            if (yval < vRaster)
                  yval = vRaster;
            }
      if (mscore->hRaster()) {
            qreal hRaster = _spatium / MScore::hRaster();
            if (xval < hRaster)
                  xval = hRaster;
            }
      // TODO: if raster, then xval/yval should be multiple of raster

      switch (key) {
            case Qt::Key_Left:
                  delta = QPointF(-xval, 0);
                  break;
            case Qt::Key_Right:
                  delta = QPointF(xval, 0);
                  break;
            case Qt::Key_Up:
                  delta = QPointF(0, -yval);
                  break;
            case Qt::Key_Down:
                  delta = QPointF(0, yval);
                  break;
            default:
                  ev->ignore();
                  return;
            }
      EditData ed;
      ed.curGrip = curGrip;
      ed.delta   = delta;
      ed.view    = this;
      ed.hRaster = mscore->hRaster();
      ed.vRaster = mscore->vRaster();
      if (curGrip != Grip::NO_GRIP && int(curGrip) < grips)
            ed.pos = grip[int(curGrip)].center() + delta;
      editObject->editDrag(ed);
      updateGrips();
      _score->update();
      mscore->endCmd();
      mscore->updateInspector();
      ev->accept();
      }

//---------------------------------------------------------
//   updateInputState
//---------------------------------------------------------

void MuseScore::updateInputState(Score* score)
      {
      InputState& is = score->inputState();
      if (is.noteEntryMode()) {
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
            case TDuration::DurationType::V_128TH:
                  getAction("pad-dot")->setChecked(false);
                  getAction("pad-dot")->setEnabled(false);
                  // fall through
            case TDuration::DurationType::V_64TH:
                  getAction("pad-dotdot")->setChecked(false);
                  getAction("pad-dotdot")->setEnabled(false);
                  // fall through
            case TDuration::DurationType::V_32ND:
                  getAction("pad-dot3")->setChecked(false);
                  getAction("pad-dot3")->setEnabled(false);
                  // fall through
            case TDuration::DurationType::V_16TH:
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

