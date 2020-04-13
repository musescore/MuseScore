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

#include "pianokeyboard.h"

#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"

namespace Ms {

const QColor colKeySelect = QColor(224, 170, 20);

const QString PianoKeyboard::pitchNames[] =
            {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};


//---------------------------------------------------------
//   PianoKeyboard
//---------------------------------------------------------

PianoKeyboard::PianoKeyboard(QWidget* parent)
   : QWidget(parent)
      {
      setMouseTracking(true);
      setAttribute(Qt::WA_NoSystemBackground);
      setAttribute(Qt::WA_StaticContents);
      setMouseTracking(true);
      yRange   = noteHeight * 128;
      curPitch = -1;
      _ypos    = 0;
      curKeyPressed = -1;
      noteHeight = DEFAULT_KEY_HEIGHT;
      _orientation = PianoOrientation::VERTICAL;
      _staff = 0;
      }



//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void PianoKeyboard::paintEvent(QPaintEvent* /*event*/)
      {
      QPainter p(this);
      p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
      
      const int fontSize = 8;
      QFont f("FreeSans", fontSize);
      p.setFont(f);

      //Check for drumset, if any
      Drumset* ds = nullptr;
      Interval transp;
      if (_staff) {
            Part* part = _staff->part();
            ds = part->instrument()->drumset();
            transp = part->instrument()->transpose();
            }

      p.setPen(QPen(Qt::black, 2));
      
      int keyboardLen = 128 * noteHeight;
      const int blackKeyLen = BLACK_KEY_WIDTH;

      const qreal whiteKeyOffset[] = {0, 1.5, 3.5, 5, 6.5, 8.5, 10.5, 12};
      const int whiteKeyDegree[] = {0, 2, 4, 5, 7, 9, 11};
      const qreal blackKeyOffset[] = {1.5, 3.5, 6.5, 8.5, 10.5};
      const int blackKeyDegree[] = {1, 3, 6, 8, 10};

      //White keys
      p.setPen(QPen(Qt::black));
      for (int midiPitch = 0; midiPitch < 128; ++midiPitch) {

            int instrPitch = midiPitch - transp.chromatic;

            int octave = instrPitch / 12;
            int degree = (instrPitch + 60) % 12;

            int key = -1;
            for (int i = 0; i < 7; ++i)
                  if (whiteKeyDegree[i] == degree) {
                        key = i;
                        break;
                        }

            if (key == -1)
                  continue;

            QString noteName = pitchNames[degree] % QString::number(octave - 1);
            if (ds)
                  noteName = ds->name(instrPitch);

            p.setBrush(curPitch == midiPitch ? colKeySelect : Qt::white);

            qreal off1 = whiteKeyOffset[key] * noteHeight + (octave * 12 + transp.chromatic) * noteHeight;
            qreal off2 = whiteKeyOffset[key + 1] * noteHeight + (octave * 12 + transp.chromatic) * noteHeight;
            if (_orientation == PianoOrientation::HORIZONTAL) {
                  QRectF rect(-_ypos + off2, 0, off2 - off1, height());
                  p.drawRect(rect);

                  if (degree == 0 && noteHeight > fontSize + 2) {
                        QRectF rectText(rect.x() + 1, rect.y(), rect.height() - 2, rect.width() - 2);
                        QTransform xform = p.transform();
                        p.rotate(90);
                        p.drawText(rectText,
                                ds ?  Qt::AlignLeft | Qt::AlignBottom
                                    : Qt::AlignRight | Qt::AlignBottom,
                                noteName);
                        p.setTransform(xform);
                        }
                  }
            else {
                  QRectF rect(0, -_ypos + keyboardLen - off2, width(), off2 - off1);

                  p.drawRect(rect);

                  if (noteHeight > fontSize + 2) {
                        if (ds) {
                              QRectF rectText(rect.x() + 1, -_ypos + keyboardLen - (instrPitch + 1) * noteHeight, rect.width() - 1, noteHeight);

                              p.drawText(rectText, Qt::AlignBottom | Qt::AlignLeft, noteName);
                              }
                        else if (degree == 0) {
                              QRectF rectText(rect.x(), rect.y(), rect.width() - 4, rect.height() - 1);
                              p.drawText(rectText, Qt::AlignRight | Qt::AlignBottom, noteName);
                              }
                        }
                  }
            }

      //Black keys
      for (int midiPitch = 0; midiPitch < 128; ++midiPitch) {
            int instrPitch = midiPitch - transp.chromatic;

            int octave = instrPitch / 12;
            int degree = instrPitch % 12;

            int key = -1;
            for (int i = 0; i < 5; ++i)
                  if (blackKeyDegree[i] == degree) {
                        key = i;
                        break;
                        }

            if (key == -1)
                  continue;

            QString noteName = pitchNames[blackKeyDegree[key]]  % QString::number(octave) + " ";
            if (ds)
                  noteName = ds->name(instrPitch);

            qreal center = blackKeyOffset[key] * noteHeight;
            qreal offset = center - noteHeight / 2.0 + (octave * 12 + transp.chromatic) * noteHeight;

            p.setPen(QPen(Qt::black));
            p.setBrush(curPitch == midiPitch ? colKeySelect : Qt::black);

            if (_orientation == PianoOrientation::HORIZONTAL) {
                  QRectF rect(offset, 0, noteHeight, blackKeyLen);

                  p.drawRect(rect);

                  if (ds) {
                        if (noteHeight > fontSize + 2) {
                              rect.setWidth(blackKeyLen);
                              rect.setHeight(noteHeight);

                              p.setPen(QPen(Qt::white));
                              p.drawText(rect, Qt::AlignLeft | Qt::AlignBottom, noteName);
                              }
                        }
                  }
            else {
                  QRectF rect(0, -_ypos + keyboardLen - offset - noteHeight, blackKeyLen, noteHeight);

                  p.drawRect(rect);

                  if (noteHeight > fontSize + 2) {
                        if (ds) {
                              p.setPen(QPen(Qt::white));
                              p.drawText(rect, Qt::AlignLeft | Qt::AlignBottom, noteName);
                              }
                        }
                  }
            }

      }

//---------------------------------------------------------
//   setYpos
//---------------------------------------------------------

void PianoKeyboard::setYpos(int val)
      {
      if (_ypos != val) {
            _ypos = val;
            update();
            }
      }

//---------------------------------------------------------
//   setNoteHeight
//---------------------------------------------------------

void PianoKeyboard::setNoteHeight(int nh)
      {
      if (noteHeight != nh) {
            noteHeight = nh;
            update();
            }
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void PianoKeyboard::setPitch(int val)
      {
      if (curPitch != val) {
            curPitch = val;
            update();
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void PianoKeyboard::mousePressEvent(QMouseEvent* event)
      {
      int offset = _orientation == PianoOrientation::HORIZONTAL
            ? event->pos().x()
            : 128 * noteHeight - (event->y() + _ypos);

      curKeyPressed = offset / noteHeight;
      if (curKeyPressed < 0 || curKeyPressed > 127)
            curKeyPressed = -1;
      
      emit keyPressed(curKeyPressed);
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void PianoKeyboard::mouseReleaseEvent(QMouseEvent*)
      {
      emit keyReleased(curKeyPressed);
      curKeyPressed = -1;
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void PianoKeyboard::mouseMoveEvent(QMouseEvent* event)
      {
      int offset = _orientation == PianoOrientation::HORIZONTAL
            ? event->pos().x()
            : 128 * noteHeight - (event->y() + _ypos);

      int pitch = offset / noteHeight;

      if (pitch != curPitch) {
            curPitch = pitch;

            //Set tooltip
            int degree = curPitch % 12;
            int octave = curPitch / 12;
            QString text = pitchNames[degree] + QString::number(octave - 1);
            Part* part = _staff->part();
            Drumset* ds = part->instrument()->drumset();
            if (ds)
                  text += " - " + ds->name(curPitch);

            setToolTip(text);

            //Send event
            emit pitchChanged(curPitch);
            if ((curKeyPressed != -1) && (curKeyPressed != pitch)) {
                  emit keyReleased(curKeyPressed);
                  curKeyPressed = pitch;
                  emit keyPressed(curKeyPressed);
                  }
            update();
            }
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void PianoKeyboard::leaveEvent(QEvent*)
      {
      if (curPitch != -1) {
            curPitch = -1;
            emit pitchChanged(-1);
            update();
            }
      }


//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianoKeyboard::setStaff(Staff* staff)
      {
      _staff = staff;
      update();
      }

//---------------------------------------------------------
//   setOrientation
//---------------------------------------------------------

void PianoKeyboard::setOrientation(PianoOrientation o)
      {
      _orientation = o;
      update();
      }
}
