//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "editinstrument.h"
#include "libmscore/instrtemplate.h"

namespace Ms {

//---------------------------------------------------------
//   EditInstrument
//---------------------------------------------------------

EditInstrument::EditInstrument(QWidget* parent)
   : QDialog(parent)
      {
      lt = new InstrumentTemplate;
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      connect(minPitch, SIGNAL(valueChanged(int)), SLOT(valueChanged()));
      connect(maxPitch, SIGNAL(valueChanged(int)), SLOT(valueChanged()));
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void EditInstrument::setInstrument(InstrumentTemplate* t)
      {
      instr = t;
//TODO      *lt   = *t;
//      nameEdit->setText(t->name);
//      shortNameEdit->setText(t->shortName);
      minPitch->setValue(t->minPitchA);
      maxPitch->setValue(t->maxPitchA);
      transposeChromatic->setValue(t->transpose.chromatic);
      transposeDiatonic->setValue(t->transpose.diatonic);
//      midiProgram->setValue(t->midiProgram);
      staves->setValue(t->nstaves());
      }

//---------------------------------------------------------
//   ~EditInstrument
//---------------------------------------------------------

EditInstrument::~EditInstrument()
      {
      delete lt;
      }

//---------------------------------------------------------
//   on_buttonCancel_pressed
//---------------------------------------------------------

void EditInstrument::on_buttonCancel_pressed()
      {
//      qDebug("cancel");
      }

//---------------------------------------------------------
//   on_buttonOk_pressed
//---------------------------------------------------------

void EditInstrument::on_buttonOk_pressed()
      {
      valueChanged();
//TODO      *instr = *lt;
      }

//---------------------------------------------------------
//   valueChanged
//---------------------------------------------------------

void EditInstrument::valueChanged()
      {
//      lt->name        = nameEdit->text();
//      lt->shortName   = shortNameEdit->text();
      lt->minPitchA    = minPitch->value();
      lt->maxPitchA    = maxPitch->value();
      lt->transpose.diatonic   = transposeDiatonic->value();
      lt->transpose.chromatic   = transposeChromatic->value();
//      lt->midiProgram = midiProgram->value();
      lt->setStaves(staves->value());
      }
}

