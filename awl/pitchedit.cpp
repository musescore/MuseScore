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
#include "utils.h"

namespace Awl {

//---------------------------------------------------------
//   PitchEdit
//---------------------------------------------------------

PitchEdit::PitchEdit(QWidget* parent)
  : QSpinBox(parent)
      {
      setRange(0, 127);
      deltaMode = false;
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void PitchEdit::keyPressEvent(QKeyEvent* ev)
      {
      if (ev->key() == Qt::Key_Return)
            emit returnPressed();
      else if (ev->key() == Qt::Key_Escape)
            emit escapePressed();
      }

//---------------------------------------------------------
//   textFromValue
//---------------------------------------------------------

QString PitchEdit::textFromValue(int v) const
      {
      if (deltaMode)
            return QString("%1").arg(v);
      else
            return pitch2string(v);
      }

//---------------------------------------------------------
//   valueFromText
//---------------------------------------------------------

int PitchEdit::valueFromText(const QString& s) const
      {
      qDebug("AwlPitchEdit::valueFromText(%s): not impl.", qPrintable(s));
      return 0;
      }

//---------------------------------------------------------
//   setDeltaMode
//---------------------------------------------------------

void PitchEdit::setDeltaMode(bool val)
      {
      deltaMode = val;
      if (deltaMode)
            setRange(-127, 127);
      else
            setRange(0, 127);
      }
}

