//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: timedialog.cpp 4391 2011-06-18 14:28:24Z wschweer $
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

extern bool useFactorySettings;

//---------------------------------------------------------
//   TimeDialog
//---------------------------------------------------------

TimeDialog::TimeDialog(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Time Signatures"));
      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      sp = MuseScore::newTimePalette();
      sp->setReadOnly(false);
      sp->setSelectable(true);
      sp->setSelected(2);

      connect(zNominal, SIGNAL(valueChanged(int)), SLOT(zChanged(int)));
      connect(nNominal, SIGNAL(valueChanged(int)), SLOT(nChanged(int)));
      connect(sp, SIGNAL(boxClicked(int)), SLOT(paletteChanged(int)));

      PaletteScrollArea* timePalette = new PaletteScrollArea(sp);
      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      timePalette->setSizePolicy(policy);

      l->addWidget(timePalette);

      _dirty = false;
      connect(addButton, SIGNAL(clicked()), SLOT(addClicked()));

      if (!useFactorySettings) {
            QFile f(dataPath + "/" + "timesigs.xml");
            if (f.exists() && sp->read(&f))
                  return;
            }
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void TimeDialog::addClicked()
      {
      TimeSig* ts = new TimeSig(gscore);
      ts->setSig(Fraction(zNominal->value(), nNominal->value()));

      // check for special text
      if ((QString("%1").arg(zNominal->value()) != zText->text())
         || (QString("%1").arg(nNominal->value()) != nText->text())) {
            ts->setNumeratorString(zText->text());
            ts->setDenominatorString(nText->text());
            }
      // extend palette:
      sp->append(ts, "");
      _dirty = true;
      sp->updateGeometry();
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void TimeDialog::save()
      {
      QDir dir;
      dir.mkpath(dataPath);
      sp->write(dataPath + "/" + "timesigs.xml");
      }

//---------------------------------------------------------
//   zChanged
//---------------------------------------------------------

void TimeDialog::zChanged(int val)
      {
      zText->setText(QString("%1").arg(val));
      }

//---------------------------------------------------------
//   nChanged
//---------------------------------------------------------

void TimeDialog::nChanged(int val)
      {
      nText->setText(QString("%1").arg(val));
      }

//---------------------------------------------------------
//   paletteChanged
//---------------------------------------------------------

void TimeDialog::paletteChanged(int idx)
      {
      printf("palette %d\n", idx);
      }

