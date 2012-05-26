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

#ifdef Q_WS_MAC
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

      if (editObject->type() == LYRICS) {
            if (cmd == "next-lyric")
                  lyricsTab(false, true, false);
            else if (cmd == "prev-lyric")
                  lyricsTab(true, true, false);
            }
#if 0
      if (editObject->type() == LYRICS) {
            int found = false;
		if (key == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  // TODO: shift+tab events are filtered by qt
                  lyricsTab(modifiers & Qt::ShiftModifier, true, false);
                  return;
                  }
            if (key == Qt::Key_Left) {
                  if (!ctrl && editObject->edit(this, curGrip, key, modifiers, s)) {
                        _score->end();
                        mscore->endCmd();
                        }
                  else
                        lyricsTab(true, true, true);      // go to previous lyrics
                  return;
                  }
            if (key == Qt::Key_Right) {
                  if (!ctrl && editObject->edit(this, curGrip, key, modifiers, s)) {
                        _score->end();
                        mscore->endCmd();
                        }
                  else
                        lyricsTab(false, false, true);    // go to next lyrics
                  found = true;
                  }
            else if (key == Qt::Key_Up) {
                  lyricsUpDown(true, true);
                  found = true;
                  }
            else if (key == Qt::Key_Down) {
                  lyricsUpDown(false, true);
                  found = true;
                  }
            else if (key == Qt::Key_Return) {
                  lyricsReturn();
                  found = true;
                  }
		else if (key == Qt::Key_Minus && !(modifiers & CONTROL_MODIFIER)) {
                  lyricsMinus();
                  found = true;
                  }
		else if (key == Qt::Key_Underscore) {
                  if (modifiers & CONTROL_MODIFIER) {
                        modifiers &= ~CONTROL_MODIFIER;
                        s = "_";
                        }
                  else {
                        lyricsUnderscore();
                        found = true;
                        }
                  }
            if (found) {
                  return;
                  }
            }
      if (editObject->type() == HARMONY) {
            if (key == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  chordTab(modifiers & Qt::ShiftModifier);
                  ev->accept();
                  return;
                  }
            }
      if (editObject->type() == FIGURED_BASS) {
            int found = false;
            if (key == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  figuredBassTab(false, modifiers & Qt::ShiftModifier);
                  found = true;
                  }
            if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
                  figuredBassTab(true, key == Qt::Key_Backtab ? true : (modifiers & Qt::ShiftModifier) );
                  found = true;
                  }
            if (key >= Qt::Key_1 && key <= Qt::Key_9 && (modifiers & CONTROL_MODIFIER)) {
                  int ticks = (MScore::division >> 4) << (key - Qt::Key_1);
                  figuredBassTicksTab(ticks);
                  found = true;
                  }
            if (found) {
                  ev->accept();
                  return;
                  }
            }
      if (!((modifiers & Qt::ShiftModifier) && (key == Qt::Key_Backtab))) {
            if (editObject->edit(this, curGrip, key, modifiers, s)) {
                  updateGrips();
                  ev->accept();
                  _score->end();
                  mscore->endCmd();
                  return;
                  }
            if (editObject->isText() && (key == Qt::Key_Left || key == Qt::Key_Right)) {
                  ev->accept();
                  _score->end();
                  mscore->endCmd();
                  //return;
                  }
            }
      QPointF delta;
      qreal _spatium = editObject->spatium();
      qreal xval     = MScore::nudgeStep * _spatium;

      if (modifiers & Qt::ControlModifier)
            xval = preferences.nudgeStep10 * _spatium;
      else if (modifiers & Qt::AltModifier)
            xval = preferences.nudgeStep50 * _spatium;
      qreal yval = xval;

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
      if (curGrip >= 0)
            ed.pos = grip[curGrip].center() + delta;
      editObject->editDrag(ed);
      updateGrips();
      _score->end();
      mscore->endCmd();
#endif
      }

//---------------------------------------------------------
//   Canvas::editKey
//---------------------------------------------------------

void ScoreView::editKey(QKeyEvent* ev)
      {
      int key                         = ev->key();
      Qt::KeyboardModifiers modifiers = ev->modifiers();
      QString s                       = ev->text();
      bool ctrl                       = modifiers == Qt::ControlModifier;

      if (MScore::debugMode)
            qDebug("keyPressEvent key 0x%02x(%c) mod 0x%04x <%s>\n",
               key, key, int(modifiers), qPrintable(s));

      if (!editObject)
            return;

      if (editObject->type() == LYRICS /*|| editObject->type() == FIGURED_BASS*/) {
            int found = false;
		if (key == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  // TODO: shift+tab events are filtered by qt
                  lyricsTab(modifiers & Qt::ShiftModifier, true, false);
                  found = true;
                  }
            else if (key == Qt::Key_Left) {
                  if (!ctrl && editObject->edit(this, curGrip, key, modifiers, s)) {
                        _score->end();
                        mscore->endCmd();
                        }
                  else
                        lyricsTab(true, true, true);      // go to previous lyrics
                  found = true;
                  }
            else if (key == Qt::Key_Right) {
                  if (!ctrl && editObject->edit(this, curGrip, key, modifiers, s)) {
                        _score->end();
                        mscore->endCmd();
                        }
                  else
                        lyricsTab(false, false, true);    // go to next lyrics
                  found = true;
                  }
            else if (key == Qt::Key_Up) {
                  lyricsUpDown(true, true);
                  found = true;
                  }
            else if (key == Qt::Key_Down) {
                  lyricsUpDown(false, true);
                  found = true;
                  }
            else if (key == Qt::Key_Return) {
                  lyricsReturn();
                  found = true;
                  }
		else if (key == Qt::Key_Minus && !(modifiers & CONTROL_MODIFIER)) {
                  lyricsMinus();
                  found = true;
                  }
		else if (key == Qt::Key_Underscore) {
                  if (modifiers & CONTROL_MODIFIER) {
                        modifiers &= ~CONTROL_MODIFIER;
                        s = "_";
                        }
                  else {
                        lyricsUnderscore();
                        found = true;
                        }
                  }
            if (found) {
                  ev->accept();
                  return;
                  }
            }
      if (editObject->type() == HARMONY) {
            if (key == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  chordTab(modifiers & Qt::ShiftModifier);
                  ev->accept();
                  return;
                  }
            }
      if (editObject->type() == FIGURED_BASS) {
            int found = false;
            if (key == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  figuredBassTab(false, modifiers & Qt::ShiftModifier);
                  found = true;
                  }
            if (key == Qt::Key_Tab || key == Qt::Key_Backtab) {
                  figuredBassTab(true, key == Qt::Key_Backtab ? true : (modifiers & Qt::ShiftModifier) );
                  found = true;
                  }
            if (key >= Qt::Key_1 && key <= Qt::Key_9 && (modifiers & CONTROL_MODIFIER)) {
                  int ticks = (MScore::division >> 4) << (key - Qt::Key_1);
                  figuredBassTicksTab(ticks);
                  found = true;
                  }
            if (found) {
                  ev->accept();
                  return;
                  }
            }
      if (!((modifiers & Qt::ShiftModifier) && (key == Qt::Key_Backtab))) {
            if (editObject->edit(this, curGrip, key, modifiers, s)) {
                  updateGrips();
                  ev->accept();
                  _score->end();
                  mscore->endCmd();
                  return;
                  }
            if (editObject->isText() && (key == Qt::Key_Left || key == Qt::Key_Right)) {
                  ev->accept();
                  _score->end();
                  mscore->endCmd();
                  //return;
                  }
            }
      QPointF delta;
      qreal _spatium = editObject->spatium();

      qreal xval, yval;
      if (editObject->type() == BEAM) {
            xval = 0.25 * _spatium;
            if (modifiers & Qt::ControlModifier)
                  xval = _spatium;
            else if (modifiers & Qt::AltModifier)
                  xval = 4 * _spatium;
            }
      else {
            xval = MScore::nudgeStep * _spatium;
            if (modifiers & Qt::ControlModifier)
                  xval = preferences.nudgeStep10 * _spatium;
            else if (modifiers & Qt::AltModifier)
                  xval = preferences.nudgeStep50 * _spatium;
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
      if (curGrip >= 0)
            ed.pos = grip[curGrip].center() + delta;
      editObject->editDrag(ed);
      updateGrips();
      _score->update();
      mscore->endCmd();
      ev->accept();
      }

//---------------------------------------------------------
//   updateInputState
//---------------------------------------------------------

void MuseScore::updateInputState(Score* score)
      {
      InputState& is = score->inputState();
      showDrumTools(is.drumset(), score->staff(is.track() / VOICES));

      getAction("pad-rest")->setChecked(is.rest);
      getAction("pad-dot")->setChecked(is.duration().dots() == 1);
      getAction("pad-dotdot")->setChecked(is.duration().dots() == 2);

      getAction("note-longa")->setChecked(is.duration()  == TDuration::V_LONG);
      getAction("note-breve")->setChecked(is.duration()  == TDuration::V_BREVE);
      getAction("pad-note-1")->setChecked(is.duration()  == TDuration::V_WHOLE);
      getAction("pad-note-2")->setChecked(is.duration()  == TDuration::V_HALF);
      getAction("pad-note-4")->setChecked(is.duration()  == TDuration::V_QUARTER);
      getAction("pad-note-8")->setChecked(is.duration()  == TDuration::V_EIGHT);
      getAction("pad-note-16")->setChecked(is.duration() == TDuration::V_16TH);
      getAction("pad-note-32")->setChecked(is.duration() == TDuration::V_32ND);
      getAction("pad-note-64")->setChecked(is.duration() == TDuration::V_64TH);
      getAction("pad-note-128")->setChecked(is.duration() == TDuration::V_128TH);

      // uncheck all voices if multi-selection
      int voice = score->selection().isSingle() ? is.voice() : -1;
      getAction("voice-1")->setChecked(voice == 0);
      getAction("voice-2")->setChecked(voice == 1);
      getAction("voice-3")->setChecked(voice == 2);
      getAction("voice-4")->setChecked(voice == 3);

      getAction("acciaccatura")->setChecked(is.noteType == NOTE_ACCIACCATURA);
      getAction("appoggiatura")->setChecked(is.noteType == NOTE_APPOGGIATURA);
      getAction("grace4")->setChecked(is.noteType  == NOTE_GRACE4);
      getAction("grace16")->setChecked(is.noteType == NOTE_GRACE16);
      getAction("grace32")->setChecked(is.noteType == NOTE_GRACE32);

      getAction("beam-start")->setChecked(is.beamMode == BEAM_BEGIN);
      getAction("beam-mid")->setChecked(is.beamMode   == BEAM_MID);
      getAction("no-beam")->setChecked(is.beamMode    == BEAM_NO);
      getAction("beam32")->setChecked(is.beamMode     == BEAM_BEGIN32);
      getAction("auto-beam")->setChecked(is.beamMode  == BEAM_AUTO);
      getAction("repitch")->setChecked(is.repitchMode());
      }

