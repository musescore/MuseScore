//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#include "clickableLabel.h"

namespace Ms {

ClickableLabel::ClickableLabel(QWidget* parent, Qt::WindowFlags f)
   : QLabel(parent)
      { Q_UNUSED(f); }

ClickableLabel::~ClickableLabel()
      {}

void ClickableLabel::mousePressEvent(QMouseEvent* event)
      {
      Q_UNUSED(event);
      emit clicked();
      }
}
