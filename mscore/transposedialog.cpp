//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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

#include "transposedialog.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/key.h"
#include "libmscore/staff.h"
#include "libmscore/harmony.h"
#include "libmscore/part.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "libmscore/keysig.h"
#include "libmscore/utils.h"
#include "libmscore/segment.h"
#include "libmscore/stafftype.h"
#include "libmscore/clef.h"

namespace Ms {

//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

TransposeDialog::TransposeDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("TransposeDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      connect(transposeByKey, SIGNAL(clicked(bool)), SLOT(transposeByKeyToggled(bool)));
      connect(transposeByInterval, SIGNAL(clicked(bool)), SLOT(transposeByIntervalToggled(bool)));

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   transposeByKeyToggled
//---------------------------------------------------------

void TransposeDialog::transposeByKeyToggled(bool val)
      {
      transposeByInterval->setChecked(!val);
      }

//---------------------------------------------------------
//   transposeByIntervalToggled
//---------------------------------------------------------

void TransposeDialog::transposeByIntervalToggled(bool val)
      {
      transposeByKey->setChecked(!val);
      }

//---------------------------------------------------------
//   mode
//---------------------------------------------------------

TransposeMode TransposeDialog::mode() const
      {
      return chromaticBox->isChecked()
                  ? (transposeByKey->isChecked() ? TransposeMode::BY_KEY : TransposeMode::BY_INTERVAL)
                  : TransposeMode::DIATONICALLY;
      }

//---------------------------------------------------------
//   enableTransposeByKey
//---------------------------------------------------------

void TransposeDialog::enableTransposeByKey(bool val)
      {
      transposeByKey->setEnabled(val);
      transposeByInterval->setChecked(!val);
      transposeByKey->setChecked(val);
      }

//---------------------------------------------------------
//   enableTransposeChordNames
//---------------------------------------------------------

void TransposeDialog::enableTransposeChordNames(bool val)
      {
      transposeChordNames->setEnabled(val);
      transposeChordNames->setChecked(!val);
      transposeChordNames->setChecked(val);
      }

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

TransposeDirection TransposeDialog::direction() const
      {
      switch(mode())
      {
      case TransposeMode::BY_KEY:
            if (closestKey->isChecked())
                  return TransposeDirection::CLOSEST;
            return upKey->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
      case TransposeMode::BY_INTERVAL:
            return upInterval->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
      case TransposeMode::DIATONICALLY:
            return upDiatonic->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
      }
      return TransposeDirection::UP;
      }


void TransposeDialog::on_chromaticBox_toggled(bool val)
{
      diatonicBox->setChecked(!val);
}

void TransposeDialog::on_diatonicBox_toggled(bool val)
{
    chromaticBox->setChecked(!val);
}

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void TransposeDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }
}

