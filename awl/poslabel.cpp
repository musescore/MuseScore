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

#include "poslabel.h"
#include "libmscore/pos.h"

namespace Awl {

//---------------------------------------------------------
//   PosLabel
//---------------------------------------------------------

PosLabel::PosLabel(TempoMap* tl, TimeSigMap* sl, QWidget* parent)
   : QLabel(parent), pos(tl, sl)
      {
      _smpte = false;
      setFrameStyle(WinPanel | Sunken);
      setLineWidth(2);
      setMidLineWidth(3);
      int fw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
      setIndent(fw);
      updateValue();
      }

PosLabel::PosLabel(QWidget* parent)
   : QLabel(parent)
      {
      _smpte = false;
      setFrameStyle(WinPanel | Sunken);
      setLineWidth(2);
      setMidLineWidth(3);
      int fw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
      setIndent(fw);
      updateValue();
      }

//---------------------------------------------------------
//   setContext
//---------------------------------------------------------

void PosLabel::setContext(TempoMap* tl, TimeSigMap* sl)
      {
      pos.setContext(tl, sl);
      }

//---------------------------------------------------------
//   sizeHint
//---------------------------------------------------------

QSize PosLabel::sizeHint() const
      {
      QFontMetrics fm(font());
      int fw = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
      int h  = fm.height() + fw * 2;
      int w;
      if (_smpte)
            w  = 2 + fm.width('9') * 9 + fm.width(':') * 3 + fw * 4;
      else
            w  = 2 + fm.width('9') * 9 + fm.width('.') * 2 + fw * 4;
      return QSize(w, h).expandedTo(QApplication::globalStrut());
      }

//---------------------------------------------------------
//   updateValue
//---------------------------------------------------------

void PosLabel::updateValue()
      {
      if (!pos.valid())
            return;
      QString s;
      if (_smpte) {
            int min, sec, frame, subframe;
            pos.msf(&min, &sec, &frame, &subframe);
            s.sprintf("%03d:%02d:%02d:%02d", min, sec, frame, subframe);
            }
      else {
            int measure, beat, tick;
            pos.mbt(&measure, &beat, &tick);
            s.sprintf("%04d.%02d.%03u", measure+1, beat+1, tick);
            }
      setText(s);
      }

//---------------------------------------------------------
//   setValue
//---------------------------------------------------------

void PosLabel::setValue(const Pos& val)
      {
      pos = val;
      setEnabled(pos.valid());
      updateValue();
      }

//---------------------------------------------------------
//   setSmpte
//---------------------------------------------------------

void PosLabel::setSmpte(bool val)
      {
      _smpte = val;
      updateValue();
      }
}

