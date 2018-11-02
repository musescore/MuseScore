//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#include "sectionbreakprop.h"
#include "libmscore/layoutbreak.h"
#include "musescore.h"

namespace Ms {

//---------------------------------------------------------
//   SectionBreakProperties
//---------------------------------------------------------

SectionBreakProperties::SectionBreakProperties(LayoutBreak* lb, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("SectionBreakProperties");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      _pause->setValue(lb->pause());
      _startWithLongNames->setChecked(lb->startWithLongNames());
      _startWithMeasureOne->setChecked(lb->startWithMeasureOne());
      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   pause
//---------------------------------------------------------

double SectionBreakProperties::pause() const
      {
      return _pause->value();
      }

//---------------------------------------------------------
//   startWithLongNames
//---------------------------------------------------------

bool SectionBreakProperties::startWithLongNames() const
      {
      return _startWithLongNames->isChecked();
      }

//---------------------------------------------------------
//   startWithMeasureOne
//---------------------------------------------------------

bool SectionBreakProperties::startWithMeasureOne() const
      {
      return _startWithMeasureOne->isChecked();
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void SectionBreakProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

