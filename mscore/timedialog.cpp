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

#include "timedialog.h"
#include "libmscore/timesig.h"
#include "palette.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/mcursor.h"
#include "libmscore/chord.h"
#include "libmscore/part.h"

namespace Ms {

extern bool useFactorySettings;

//---------------------------------------------------------
//   TimeDialog
//---------------------------------------------------------

TimeDialog::TimeDialog(QWidget* parent)
   : QWidget(parent, Qt::WindowFlags(Qt::Dialog | Qt::Window))
      {
      setupUi(this);
      setWindowTitle(tr("Time Signatures"));

      QLayout* l = new QVBoxLayout();
      l->setContentsMargins(0, 0, 0, 0);
      frame->setLayout(l);

      sp = MuseScore::newTimePalette();
      sp->setReadOnly(false);
      sp->setSelectable(true);

      // (should be in sync with validator in timeproperties.cpp and inspector.cpp)
      QRegExp rx("[0-9+CO()|=,x ~X\\*\\-\\[\\]\\.\\/\\x00A2\\x00D8\\x00BC\\x00BD\\x00BE\\xE097\\xE098\\xE099\\xE09A\\xE09B\\xE09C\\xE09D]*");
      QValidator *validator = new QRegExpValidator(rx, this);
      //zText->setValidator(validator);
      pText->setValidator(validator);


      connect(zNominal,  SIGNAL(valueChanged(int)), SLOT(zChanged(int)));
      connect(nNominal,  SIGNAL(currentIndexChanged(int)), SLOT(nChanged(int)));
      connect(sp,        SIGNAL(boxClicked(int)),   SLOT(paletteChanged(int)));
      connect(sp,        SIGNAL(changed()),         SLOT(setDirty()));
      connect(addButton, SIGNAL(clicked()),         SLOT(addClicked()));
      connect(pText,     SIGNAL(textChanged(const QString&)),    SLOT(textChanged()));

      _timePalette = new PaletteScrollArea(sp);
      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      _timePalette->setSizePolicy(policy);
      _timePalette->setRestrictHeight(false);

      l->addWidget(_timePalette);

      _dirty = false;

      if (useFactorySettings || !sp->read(dataPath + "/timesigs")) {
            Fraction sig(4,4);
            groups->setSig(sig, Groups::endings(sig), pText->text());
            }
      for (int i = 0; i < sp->size(); ++i)      // cells can be changed
            sp->setCellReadOnly(i, false);

      sp->element(2)->layout();
      sp->setSelected(2);
      paletteChanged(2);
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void TimeDialog::addClicked()
      {
      TimeSig* ts = new TimeSig(gscore);
      ts->setSig(Fraction(zNominal->value(), denominator()));
      ts->setGroups(groups->groups());

      // check for special text
      if (!pText->text().isEmpty())
            ts->setParserString(pText->text());
      // extend palette:
      sp->append(ts, "");
      sp->setSelected(sp->size() - 1);
      _dirty = true;
      emit timeSigAdded(ts);
      }

//---------------------------------------------------------
//   showTimePalette
//---------------------------------------------------------

void TimeDialog::showTimePalette(bool val)
      {
      _timePalette->setVisible(val);
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void TimeDialog::save()
      {
      QDir dir;
      dir.mkpath(dataPath);
      sp->write(dataPath + "/timesigs");
      qDebug("TimeDialog::save(): %s", qPrintable(dataPath+"/timesigs"));
      }

//---------------------------------------------------------
//   zChanged
//---------------------------------------------------------

void TimeDialog::zChanged(int val)
      {
      Q_UNUSED(val);
      Fraction sig(zNominal->value(), denominator());
      groups->setSig(sig, Groups::endings(sig), pText->text());
      }

//---------------------------------------------------------
//   nChanged
//---------------------------------------------------------

void TimeDialog::nChanged(int val)
      {
      Q_UNUSED(val);
      Fraction sig(zNominal->value(), denominator());
      groups->setSig(sig, Groups::endings(sig), pText->text());
      }

//---------------------------------------------------------
//   denominator2Idx
//---------------------------------------------------------

int TimeDialog::denominator2Idx(int denominator) const
      {
      int val = 4;
      switch (denominator) {
            case 1:  val = 0; break;
            case 2:  val = 1; break;
            case 4:  val = 2; break;
            case 8:  val = 3; break;
            case 16: val = 4; break;
            case 32: val = 5; break;
            case 64: val = 6; break;
            }
      return val;
      }

//---------------------------------------------------------
//   denominator
//---------------------------------------------------------

int TimeDialog::denominator() const
      {
      int val = 4;
      switch (nNominal->currentIndex()) {
            case 0: val = 1; break;
            case 1: val = 2; break;
            case 2: val = 4; break;
            case 3: val = 8; break;
            case 4: val = 16; break;
            case 5: val = 32; break;
            case 6: val = 64; break;
            }
      return val;
      }

//---------------------------------------------------------
//   paletteChanged
//---------------------------------------------------------

void TimeDialog::paletteChanged(int idx)
      {
      TimeSig* e = static_cast<TimeSig*>(sp->element(idx));
      if (!e || e->type() != ElementType::TIMESIG) {
            zNominal->setEnabled(false);
            nNominal->setEnabled(false);
            pText->setEnabled(false);
            groups->setEnabled(false);
            addButton->setEnabled(false);
            return;
            }
      zNominal->setEnabled(true);
      nNominal->setEnabled(true);
      pText->setEnabled(true);
      groups->setEnabled(true);
      addButton->setEnabled(true);

      Fraction sig(e->sig());
      Groups g = e->groups();
      if (g.empty())
            g = Groups::endings(sig);
      zNominal->setValue(sig.numerator());
      nNominal->setCurrentIndex(denominator2Idx(sig.denominator()));
      pText->setText(e->parserString());
      groups->setSig(sig, g, pText->text());
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void TimeDialog::textChanged()
      {
      Fraction sig(zNominal->value(), denominator());
      groups->setSig(sig, Groups::endings(sig), pText->text());
      }


}

