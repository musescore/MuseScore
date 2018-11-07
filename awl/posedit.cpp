//=============================================================================
//  Awl
//  Audio Widget Library
//
//  Copyright (C) 2002-2006 by Werner Schweer and others
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

#include "al/al.h"
#include "awl.h"
#include "posedit.h"
#include "al/sig.h"

namespace Awl {

//---------------------------------------------------------
//   PosEdit
//---------------------------------------------------------

PosEdit::PosEdit(QWidget* parent)
   : QAbstractSpinBox(parent)
      {
      initialized = false;
      setReadOnly(false);
      setSmpte(false);
      }

void* PosEdit::operator new(size_t n)
      {
      void* p = new char[n];
      memset(p, 0, n);
      return p;
      }

PosEdit::~PosEdit()
      {
      }

QSize PosEdit::sizeHint() const
	{
      QFontMetrics fm(font());
      int fw = style()->pixelMetric(QStyle::PM_SpinBoxFrameWidth);
      int h  = fm.height() + fw * 2;
      int w = fw * 4 + 10;	// HACK: 10 = spinbox up/down arrows
      if (_smpte)
            w  += 2 + fm.width('9') * 9 + fm.width(':') * 3 + fw * 4;
      else
            w  += 2 + fm.width('9') * 9 + fm.width('.') * 2 + fw * 4;
      return QSize(w, h).expandedTo(QApplication::globalStrut());
	}

//---------------------------------------------------------
//   event
//    filter Tab and Backtab key events
//---------------------------------------------------------

bool PosEdit::event(QEvent* event)
      {
      if (event->type() == QEvent::KeyPress) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            int segment = curSegment();
            if (ke->key() == Qt::Key_Backtab) {
                  if (_smpte) {
                        if (segment == 3) {
                              lineEdit()->setSelection(7, 2);
                              return true;
                              }
                        else if (segment == 2) {
                              lineEdit()->setSelection(4, 2);
                              return true;
                              }
                        else if (segment == 1) {
                              lineEdit()->setSelection(0, 3);
                              return true;
                              }
                        }
                  else {
                        if (segment == 2) {
                              lineEdit()->setSelection(5, 2);
                              return true;
                              }
                        if (segment == 1) {
                              lineEdit()->setSelection(0, 4);
                              return true;
                              }
                        }
                  }
            if (ke->key() == Qt::Key_Tab) {
                  if (_smpte) {
                        if (segment == 0) {
                              lineEdit()->setSelection(4, 2);
                              return true;
                              }
                        else if (segment == 1) {
                              lineEdit()->setSelection(7, 2);
                              return true;
                              }
                        else if (segment == 2) {
                              lineEdit()->setSelection(10, 2);
                              return true;
                              }
                        }
                  else {
                        if (segment == 0) {
                              lineEdit()->setSelection(5, 2);
                              return true;
                              }
                        if (segment == 1) {
                              lineEdit()->setSelection(8, 3);
                              return true;
                              }
                        }
                  }
            }
      else if (event->type() == QEvent::FocusIn) {
            QFocusEvent* fe = static_cast<QFocusEvent*>(event);
            QAbstractSpinBox::focusInEvent(fe);
            int segment = curSegment();
            switch(segment) {
                  case 0:  lineEdit()->setSelection(0,4); break;
                  case 1:  lineEdit()->setSelection(5,2); break;
                  case 2:  lineEdit()->setSelection(8,3); break;
                  }
            return true;
            }
      return QAbstractSpinBox::event(event);
      }

//---------------------------------------------------------
//   setSmpte
//---------------------------------------------------------

void PosEdit::setSmpte(bool f)
      {
      _smpte = f;
      if (_smpte)
            lineEdit()->setInputMask("999:99:99:99");
      else
            lineEdit()->setInputMask("9999.99.999");
      updateValue();
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PosEdit::setValue(const Pos& time)
      {
      _pos = time;
      updateValue();
      }

void PosEdit::setValue(const QString& s)
      {
      Pos time(s);
      setValue(time);
      }

void PosEdit::setValue(int t)
      {
      Pos time(t);
      setValue(time);
      }

//---------------------------------------------------------
//   updateValue
//---------------------------------------------------------

void PosEdit::updateValue()
      {
      char buffer[64];
      if (_smpte) {
            int minute, sec, frame, subframe;
            _pos.msf(&minute, &sec, &frame, &subframe);
            sprintf(buffer, "%03d:%02d:%02d:%02d", minute, sec, frame, subframe);
            }
      else {
            int bar, beat;
            int tick;
            _pos.mbt(&bar, &beat, &tick);
            sprintf(buffer, "%04d.%02d.%03d", bar+1, beat+1, tick);
            }
      lineEdit()->setText(buffer);
      }

//---------------------------------------------------------
//   stepEnables
//---------------------------------------------------------

QAbstractSpinBox::StepEnabled PosEdit::stepEnabled() const
      {
      int segment = curSegment();
      QAbstractSpinBox::StepEnabled en = QAbstractSpinBox::StepUpEnabled | QAbstractSpinBox::StepDownEnabled;

      if (_smpte) {
             int minute, sec, frame, subframe;
            _pos.msf(&minute, &sec, &frame, &subframe);
            switch(segment) {
                  case 0:
                        if (minute == 0)
                              en &= ~QAbstractSpinBox::StepDownEnabled;
                        break;
                  case 1:
                        if (sec == 0)
                              en &= ~QAbstractSpinBox::StepDownEnabled;
                        else if (sec == 59)
                              en &= ~QAbstractSpinBox::StepUpEnabled;
                        break;
                  case 2:
                        if (frame == 0)
                              en &= ~QAbstractSpinBox::StepDownEnabled;
                        else if (frame == 23)
                              en &= ~QAbstractSpinBox::StepUpEnabled;
                        break;
                  case 3:
                        if (subframe == 0)
                              en &= ~QAbstractSpinBox::StepDownEnabled;
                        else if (subframe == 99)
                              en &= ~QAbstractSpinBox::StepUpEnabled;
                        break;
                  }
            }
      else {
            int bar, beat;
            unsigned tick;
            sigmap.tickValues(_pos.tick(), &bar, &beat, &tick);
            unsigned tb = sigmap.ticksBeat(_pos.tick());
            unsigned tm = sigmap.ticksMeasure(_pos.tick());
            int bm = tm / tb;

            switch (segment) {
                  case 0:
                        if (bar == 0)
                              en &= ~QAbstractSpinBox::StepDownEnabled;
                        break;
                  case 1:
                        if (beat == 0)
                              en &= ~QAbstractSpinBox::StepDownEnabled;
                        else {
                              if (beat >= (bm-1))
                                    en &= ~QAbstractSpinBox::StepUpEnabled;
                              }
                        break;
                  case 2:
                        if (tick == 0)
                              en &= ~QAbstractSpinBox::StepDownEnabled;
                        else {
                              if (tick >= (tb-1))
                                    en &= ~QAbstractSpinBox::StepUpEnabled;
                              }
                        break;
                  }
            }
      return en;
      }

//---------------------------------------------------------
//   fixup
//---------------------------------------------------------

void PosEdit::fixup(QString& input) const
      {
      printf("fixup <%s>\n", input.toLatin1().data());
      }

//---------------------------------------------------------
//   validate
//---------------------------------------------------------

QValidator::State PosEdit::validate(QString&,int&) const
      {
      // TODO
//      printf("validate\n");
      return QValidator::Acceptable;
      }

//---------------------------------------------------------
//   curSegment
//---------------------------------------------------------

int PosEdit::curSegment() const
      {
      QLineEdit* le = lineEdit();
      int pos = le->cursorPosition();
      int segment = -1;

      if (_smpte) {
            if (pos >= 0 && pos <= 3)
                  segment = 0;
            else if (pos >= 4 && pos <= 6)
                  segment = 1;
            else if (pos >= 7 && pos <= 9)
                  segment = 2;
            else if (pos >= 10)
                  segment = 3;
            }
      else {
            if (pos >= 0 && pos <= 4)
                  segment = 0;
            else if (pos >= 5 && pos <= 7)
                  segment = 1;
            else if (pos >= 8)
                  segment = 2;
            else
                  printf("curSegment = -1, pos %d\n", pos);
            }
      return segment;
      }

//---------------------------------------------------------
//   stepBy
//---------------------------------------------------------

void PosEdit::stepBy(int steps)
      {
      int segment = curSegment();
      int selPos;
      int selLen;

      bool changed = false;

      if (_smpte) {
             int minute, sec, frame, subframe;
            _pos.msf(&minute, &sec, &frame, &subframe);
            switch(segment) {
                  case 0:
                        minute += steps;
                        if (minute < 0)
                              minute = 0;
                        selPos = 0;
                        selLen = 3;
                        break;
                  case 1:
                        sec += steps;
                        if (sec < 0)
                              sec = 0;
                        if (sec > 59)
                              sec = 59;
                        selPos = 4;
                        selLen = 2;
                        break;
                  case 2:
                        frame += steps;
                        if (frame < 0)
                              frame = 0;
                        if (frame > 24)         //TD frame type?
                              frame = 24;
                        selPos = 7;
                        selLen = 2;
                        break;
                  case 3:
                        subframe += steps;
                        if (subframe < 0)
                              subframe = 0;
                        if (subframe > 99)
                              subframe = 99;
                        selPos = 10;
                        selLen = 2;
                        break;
                  default:
                        return;
                  }
            Pos newPos(minute, sec, frame, subframe);
            if (!(newPos == _pos)) {
                  changed = true;
                  _pos = newPos;
                  }
            }
      else {
            int bar, beat, tick;
            _pos.mbt(&bar, &beat, &tick);

            int tb = sigmap.ticksBeat(_pos.tick());
            unsigned tm = sigmap.ticksMeasure(_pos.tick());
            int bm = tm / tb;

            switch(segment) {
                  case 0:
                        bar += steps;
                        if (bar < 0)
                              bar = 0;
                        selPos = 0;
                        selLen = 4;
                        break;
                  case 1:
                        beat += steps;
                        if (beat < 0)
                              beat = 0;
                        else if (beat >= bm)
                              beat = bm - 1;
                        selPos = 5;
                        selLen = 2;
                        break;
                  case 2:
                        tick += steps;
                        if (tick < 0)
                              tick = 0;
                        else if (tick >= tb)
                              tick = tb -1;
                        selPos = 8;
                        selLen = 3;
                        break;
                  default:
                        return;
                  }
            Pos newPos(bar, beat, tick);
            if (!(newPos == _pos)) {
                  changed = true;
                  _pos = newPos;
                  }
            }
      if (changed) {
            updateValue();
            emit valueChanged(_pos);
            }
      lineEdit()->setSelection(selPos, selLen);
      }

      void PosEdit::paintEvent(QPaintEvent* event) {
            if (!initialized)
                  updateValue();
            initialized = true;
            QAbstractSpinBox::paintEvent(event);
            }
}

