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

#include "pianolevels.h"

#include "pianoruler.h"
#include "pianokeyboard.h"
#include "pianoview.h"
#include "pianolevelsfilter.h"
#include "preferences.h"
#include "libmscore/segment.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/note.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/tuplet.h"
#include "libmscore/noteevent.h"

namespace Ms {

//---------------------------------------------------------
//   PianoLevels
//---------------------------------------------------------

PianoLevels::PianoLevels(QWidget *parent)
    : QWidget(parent)
       {
       setMouseTracking(true);
       _xpos   = 0;
       _xZoom = X_ZOOM_INITIAL;
       _tuplet = 1;
       _subdiv = 0;
       _levelsIndex = 0;
       minBeatGap = 20;
       vMargin = 10;
       levelLen = 20;
       mouseDown = false;
       dragging = false;
       }

//---------------------------------------------------------
//   ~PianoLevels
//---------------------------------------------------------

PianoLevels::~PianoLevels()
      {
      clearNoteData();
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void PianoLevels::setScore(Score* s, Pos* lc)
      {
      _score = s;
      _locator = lc;
      if (_score)
            _cursor.setContext(_score->tempomap(), _score->sigmap());
      setEnabled(_score != 0);
      }

//---------------------------------------------------------
//   setXpos
//---------------------------------------------------------

void PianoLevels::setXpos(int val)
      {
      _xpos = val;
      update();
      }


//---------------------------------------------------------
//   pixelXToTick
//---------------------------------------------------------

int PianoLevels::pixelXToTick(int pixX) {
      return (int)((pixX + _xpos) / _xZoom) - MAP_OFFSET;
      }


//---------------------------------------------------------
//   tickToPixelX
//---------------------------------------------------------

int PianoLevels::tickToPixelX(int tick) {
      return (int)(tick + MAP_OFFSET) * _xZoom - _xpos;
      }


//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void PianoLevels::paintEvent(QPaintEvent* e)
      {
      QPainter p(this);

      QColor colPianoBg;
      QColor noteDeselected;
      QColor noteSelected;

      QColor colGridLine;
      QColor colText;

      switch (preferences.globalStyle()) {
            case MuseScoreStyleType::DARK_FUSION:
                  colPianoBg = QColor(preferences.getColor(PREF_UI_PIANOROLL_DARK_BG_BASE_COLOR));
                  noteDeselected = QColor(preferences.getColor(PREF_UI_PIANOROLL_DARK_NOTE_UNSEL_COLOR));
                  noteSelected = QColor(preferences.getColor(PREF_UI_PIANOROLL_DARK_NOTE_SEL_COLOR));

                  colGridLine = QColor(preferences.getColor(PREF_UI_PIANOROLL_DARK_BG_GRIDLINE_COLOR));
                  colText = QColor(preferences.getColor(PREF_UI_PIANOROLL_DARK_BG_TEXT_COLOR));
                  break;
            default:
                  colPianoBg = QColor(preferences.getColor(PREF_UI_PIANOROLL_LIGHT_BG_BASE_COLOR));
                  noteDeselected = QColor(preferences.getColor(PREF_UI_PIANOROLL_LIGHT_NOTE_UNSEL_COLOR));
                  noteSelected = QColor(preferences.getColor(PREF_UI_PIANOROLL_LIGHT_NOTE_SEL_COLOR));

                  colGridLine = QColor(preferences.getColor(PREF_UI_PIANOROLL_LIGHT_BG_GRIDLINE_COLOR));
                  colText = QColor(preferences.getColor(PREF_UI_PIANOROLL_LIGHT_BG_TEXT_COLOR));
                  break;
            }


      const QPen penLineMajor = QPen(colGridLine, 2.0, Qt::SolidLine);
      const QPen penLineMinor = QPen(colGridLine, 1.0, Qt::SolidLine);
      const QPen penLineSub = QPen(colGridLine, 1.0, Qt::DotLine);

      const QRect& r = e->rect();

      p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

      p.setBrush(colPianoBg);
      p.drawRect(0, 0, width(), height());

      if (!_score)
            return;

      Pos pos1(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(r.x()), 0), TType::TICKS);
      Pos pos2(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(r.x() + r.width()), 0), TType::TICKS);

      //draw vert lines
      int bar1, bar2, beat, tick;

      pos1.mbt(&bar1, &beat, &tick);
      pos2.mbt(&bar2, &beat, &tick);

      //Estimate bar width since changing time signatures can make this inconsistent.
      // Assuming 480 ticks per beat, 4 beats per bar
      qreal pixPerBar = MScore::division * 4 * _xZoom;
      qreal pixPerBeat = MScore::division * _xZoom;

      int barSkip = ceil(minBeatGap / pixPerBar);
      barSkip = (int)pow(2, ceil(log(barSkip)/log(2)));

      int beatSkip = ceil(minBeatGap / pixPerBeat);
      beatSkip = (int)pow(2, ceil(log(beatSkip)/log(2)));

      //Round down to first bar to be a multiple of barSkip
      bar1 = (bar1 / barSkip) * barSkip;

//      int subExp = qMin((int)floor(log2(pixPerBeat / minBeatGap)), _subBeats);
//      int numSubBeats = pow(2, subExp);

      for (int bar = bar1; bar <= bar2; bar += barSkip) {
            Pos stick(_score->tempomap(), _score->sigmap(), bar, 0, 0);

            SigEvent sig = stick.timesig();
            int z = sig.timesig().numerator();
            for (int beat1 = 0; beat1 < z; beat1 += beatSkip) {
                  Pos beatPos(_score->tempomap(), _score->sigmap(), bar, beat1, 0);
                  double xp = tickToPixelX(beatPos.time(TType::TICKS));
                  if (xp < 0)
                        continue;

                  if (beat1 == 0) {
                        p.setPen(penLineMajor);
                        }
                  else {
                        p.setPen(penLineMinor);
                        }

                  p.drawLine(xp, 0, xp, height());

                  int subbeats = _tuplet * (1 << _subdiv);

                  for (int sub = 1; sub < subbeats; ++sub) {
                        Pos subBeatPos(_score->tempomap(), _score->sigmap(), bar, beat1, sub * MScore::division / subbeats);
                        xp = tickToPixelX(subBeatPos.time(TType::TICKS));

                        p.setPen(penLineSub);
                        p.drawLine(xp, 0, xp, height());
                        }
                  }
            }


      //draw horiz lines
      PianoLevelsFilter* filter = PianoLevelsFilter::FILTER_LIST[_levelsIndex];

      QFont f("FreeSans", 7);
      p.setFont(f);

      int div = filter->divisionGap();
      int minGuide = (int)floor(filter->minRange() / (qreal)div);
      int maxGuide = (int)ceil(filter->maxRange() / (qreal)div);

      for (int i = minGuide; i <= maxGuide; ++i) {
            p.setPen(i == 0 || i == minGuide || i == maxGuide ? penLineMajor : penLineMinor);

            int y = valToPixelY(i * div);
            p.drawLine(0, y, width(), y);

            //labels
            QRectF textRect(2, y - 12, width() - 2, 12);
            p.setPen(QPen(colText));
            p.drawText(textRect,
                  Qt::AlignLeft | Qt::AlignBottom, QString::number(i * div));
            }


      //Note lines
      p.setBrush(Qt::NoBrush);
      int pix0 = valToPixelY(0);

      for (int i = 0; i < noteList.size(); ++i) {
            Note* note = noteList[i];
            if (filter->isPerEvent()) {
                  for (NoteEvent& ne : note->playEvents()) {
                        int x = tickToPixelX(noteStartTick(note, &ne));

                        int val = filter->value(_staff, note, &ne);
                        p.setPen(QPen(note->selected() ? noteSelected : noteDeselected, 2));
                        int pixY = valToPixelY(val);
                        p.drawLine(x, pix0, x, pixY);

                        //hbar
                        p.setPen(QPen(note->selected() ? noteSelected : noteDeselected, 2));
                        p.drawLine(x, pixY, x + levelLen, pixY);
                        p.drawEllipse(x - 1, pixY - 1, 3, 3);
                        }

                  }
            else {
                  int x = tickToPixelX(noteStartTick(note, 0));

                  int val = filter->value(_staff, note, 0);

                  p.setPen(QPen(note->selected() ? noteSelected : noteDeselected, 2));
                  int pixY = valToPixelY(val);
                  p.drawLine(x, pix0, x, pixY);

                  //hbar
                  p.setPen(QPen(note->selected() ? noteSelected : noteDeselected, 2));
                  p.drawLine(x, pixY, x + levelLen, pixY);
                  p.drawEllipse(x - 1, pixY - 1, 3, 3);
                  }
            }
      }


//---------------------------------------------------------
//   noteStartTick
//---------------------------------------------------------

int PianoLevels::noteStartTick(Note* note, NoteEvent* evt) {
      Chord* chord = note->chord();
      int ticks = chord->duration().ticks();

      return note->chord()->tick()
            + (evt ? evt->ontime() * ticks / 1000 : 0);
      }


//---------------------------------------------------------
//   valToPixelY
//---------------------------------------------------------

int PianoLevels::valToPixelY(int value) {
      PianoLevelsFilter* filter = PianoLevelsFilter::FILTER_LIST[_levelsIndex];

      int range = filter->maxRange() - filter->minRange();
      qreal frac = (value - filter->minRange()) / (qreal)range;

      return (int)(height() - vMargin * 2) * (1 - frac) + vMargin;
      }


//---------------------------------------------------------
//   pixelYToVal
//---------------------------------------------------------

int PianoLevels::pixelYToVal(int pix) {
      qreal frac = 1 - (pix - vMargin) / (qreal)(height() - vMargin * 2);

      PianoLevelsFilter* filter = PianoLevelsFilter::FILTER_LIST[_levelsIndex];
      int range = filter->maxRange() - filter->minRange();
      return (int)(frac * range + filter->minRange());
      }


//---------------------------------------------------------
//   mousePressEvent
//       For all points between tick0 and tick1, linearly interploate between value0 and value1 and
//       use it to set the value of the level.
//---------------------------------------------------------

void PianoLevels::adjustLevel(int tick0, int value0, int tick1, int value1, bool selectedOnly)
      {
      if (tick1 < tick0) {
            int tmp = tick0;
            tick0 = tick1;
            tick1 = tmp;

            tmp = value0;
            value0 = value1;
            value1 = tmp;
            }

      PianoLevelsFilter* filter = PianoLevelsFilter::FILTER_LIST[_levelsIndex];
      bool hitNote = false;

      for (int i = 0; i < noteList.size(); ++i) {
            Note* note = noteList[i];
            if (selectedOnly && !note->selected())
                  continue;

            if (filter->isPerEvent()) {
                  for (NoteEvent& e : note->playEvents()) {
                        int tick = noteStartTick(note, &e);
                        if (tick0 <= tick && tick <= tick1) {
                              int value = tick0 == tick1 ? value0
                                    : (value1 - value0) * (tick - tick0) / (tick1 - tick0) + value0;

                              filter->setValue(_staff, note, &e, value);
                              hitNote = true;
                              }
                        }
                  }
            else {
                  int tick = noteStartTick(note, 0);
                  if (tick0 <= tick && tick <= tick1) {
                        int value = (value1 - value0) * (tick - tick0) / (tick1 - tick0) + value0;
                        filter->setValue(_staff, note, 0, value);
                        hitNote = true;
                        }
                  }
            }

      if (hitNote) {
            update();
            emit noteLevelsChanged();
            }

      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void PianoLevels::mousePressEvent(QMouseEvent* e)
      {
      if (e->button() == Qt::LeftButton) {
            mouseDown = true;
            mouseDownPos = e->pos();
            lastMousePos = mouseDownPos;
            }
      }


//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void PianoLevels::mouseReleaseEvent(QMouseEvent* e)
      {
      if (e->button() == Qt::LeftButton) {

            if (!dragging) {
                  //Handle click
                  lastMousePos = e->pos();

                  int tick0 = pixelXToTick(lastMousePos.x() - 4);
                  int tick1 = pixelXToTick(lastMousePos.x() + 4);
                  int val = pixelYToVal(lastMousePos.y());
                  adjustLevel(tick0, val, tick1, val);
            }

            mouseDown = false;
            dragging = false;
            }
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void PianoLevels::mouseMoveEvent(QMouseEvent* e)
      {
      int modifiers = QGuiApplication::keyboardModifiers();
      bool bnShift = modifiers & Qt::ShiftModifier;

      if (mouseDown) {
            if (!dragging) {
                  int dx = e->x() - mouseDownPos.x();
                  int dy = e->y() - mouseDownPos.y();
                  if (dx * dx + dy * dy > 4) {
                        //Start dragging
                        dragging = true;
                        }
                  }

            if (dragging) {
                  int tick0 = pixelXToTick(lastMousePos.x());
                  int tick1 = pixelXToTick(e->x());

                  int val0;
                  int val1;

                  if (bnShift) {
                        //If shift is held, set to value at mousedown
                        val0 = pixelYToVal(mouseDownPos.y());
                        val1 = pixelYToVal(mouseDownPos.y());
                        }
                  else {
                        val0 = pixelYToVal(lastMousePos.y());
                        val1 = pixelYToVal(e->y());
                        }

                  lastMousePos = e->pos();
                  adjustLevel(tick0, val0, tick1, val1);
                  }

            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianoLevels::moveLocator(QMouseEvent* e)
      {
      Pos pos(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(e->pos().x()), 0), TType::TICKS);
      if (e->buttons() & Qt::LeftButton)
            emit locatorMoved(0, pos);
      else if (e->buttons() & Qt::MidButton)
            emit locatorMoved(1, pos);
      else if (e->buttons() & Qt::RightButton)
            emit locatorMoved(2, pos);
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void PianoLevels::leaveEvent(QEvent*)
      {
      _cursor.setInvalid();
      emit posChanged(_cursor);
      update();
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void PianoLevels::setPos(const Pos& pos)
      {
      if (_cursor != pos) {
            _cursor = pos;
            update();
            }
      }

//---------------------------------------------------------
//   setXZoom
//---------------------------------------------------------

void PianoLevels::setXZoom(qreal xZoom)
      {
      _xZoom = xZoom;
      update();
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianoLevels::setStaff(Staff* s, Pos* l)
      {
      _locator = l;

      if (_staff == s)
            return;

      _staff    = s;
      updateNotes();
      }


//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

void PianoLevels::addChord(Chord* chord, int voice)
      {
      for (Chord* c : chord->graceNotes())
            addChord(c, voice);
      for (Note* note : chord->notes()) {
            if (note->tieBack())
                  continue;
            noteList.append(note);
            }
      }

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void PianoLevels::updateNotes()
      {
      clearNoteData();

      if (!_staff) {
            return;
            }

      int staffIdx   = _staff->idx();

      SegmentType st = SegmentType::ChordRest;
      for (Segment* s = _staff->score()->firstSegment(st); s; s = s->next1(st)) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  int track = voice + staffIdx * VOICES;
                  Element* e = s->element(track);
                  if (e && e->isChord())
                        addChord(toChord(e), voice);
                  }
            }

      update();
      }

//---------------------------------------------------------
//   clearNoteData
//---------------------------------------------------------

void PianoLevels::clearNoteData()
      {
      noteList.clear();
      }

//---------------------------------------------------------
//   setTuplet
//---------------------------------------------------------

void PianoLevels::setTuplet(int value)
      {
      if (_tuplet != value) {
            _tuplet = value;
            update();
            emit tupletChanged(_tuplet);
            }
      }

//---------------------------------------------------------
//   setSubdiv
//---------------------------------------------------------

void PianoLevels::setSubdiv(int value)
      {
      if (_subdiv != value) {
            _subdiv = value;
            update();
            emit subdivChanged(_subdiv);
            }
      }

//---------------------------------------------------------
//   setLevelsIndex
//---------------------------------------------------------

void PianoLevels::setLevelsIndex(int index)
      {
      if (_levelsIndex != index) {
            _levelsIndex = index;
            update();
            emit levelsIndexChanged(_levelsIndex);
            }
      }

}
