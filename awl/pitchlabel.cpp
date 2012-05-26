//=============================================================================
//  Awl
//  Audio Widget Library
//  $Id:$
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

#include "pitchedit.h"
#include "pitchlabel.h"
#include "utils.h"

namespace Awl {

//---------------------------------------------------------
//   PitchLabel
//---------------------------------------------------------

PitchLabel::PitchLabel(QWidget* parent)
   : QLabel(parent)
      {
      _pitchMode = true;
      _value = -1;
      setFrameStyle(WinPanel | Sunken);
      setLineWidth(2);
      setMidLineWidth(3);
      setValue(0);
      int fw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
      setIndent(fw);
      }

//---------------------------------------------------------
//   setPitchMode
//---------------------------------------------------------

void PitchLabel::setPitchMode(bool val)
      {
      _pitchMode = val;
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize PitchLabel::sizeHint() const
      {
      QFontMetrics fm(font());
      int fw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
      int h  = fm.height() + fw * 2;
//      int w = 2 + fm.width(QString("A#8")) +  fw * 4;
      int w = 2 + fm.width(QString("-9999")) + fw * 4;     // must display 14Bit controller values
      return QSize(w, h).expandedTo(QApplication::globalStrut());
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PitchLabel::setValue(int val)
      {
      if (val == _value)
            return;
      _value = val;
      QString s;
      if (_pitchMode)
            s = pitch2string(_value);
      else
            s.sprintf("%d", _value);
      setText(s);
      }

//---------------------------------------------------------
//   setInt
//---------------------------------------------------------

void PitchLabel::setInt(int val)
      {
      if (_pitchMode)
            setPitchMode(false);
      setValue(val);
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void PitchLabel::setPitch(int val)
      {
      if (!_pitchMode)
            setPitchMode(true);
      setEnabled(val != -1);
      setValue(val);
      }
}

