//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: magbox.cpp 5568 2012-04-22 10:08:43Z wschweer $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "magbox.h"
#include "scoreview.h"
#include "libmscore/page.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/mscore.h"

namespace Ms {

static MagIdx startMag = MagIdx::MAG_100;

//---------------------------------------------------------
//   magTable
//    list of strings shown in QComboBox "MagBox"
//---------------------------------------------------------

static const char* magTable[] = {
     "25%", "50%", "75%", "100%", "150%", "200%", "400%", "800%", "1600%",
      QT_TRANSLATE_NOOP("magTable","Page Width"),
      QT_TRANSLATE_NOOP("magTable","Whole Page"),
      QT_TRANSLATE_NOOP("magTable","Two Pages"),
      ""
     };

//---------------------------------------------------------
//   MagBox
//---------------------------------------------------------

MagBox::MagBox(QWidget* parent)
   : QComboBox(parent)
      {
      freeMag = 1.0;
      setEditable(true);
      setInsertPolicy(QComboBox::InsertAtBottom);
      setToolTip(tr("Zoom"));
      setWhatsThis(tr("Zoom"));
      setValidator(new MagValidator(this));

      unsigned int n = sizeof(magTable)/sizeof(*magTable) - 1;
      for (unsigned int i =  0; i < n; ++i) {
            QString ts(QCoreApplication::translate("magTable", magTable[i]));
            addItem(ts, i);
            if (MagIdx(i) == startMag)
                  setCurrentIndex(i);
            }
      addItem(QString("%1%").arg(freeMag * 100), int(MagIdx::MAG_FREE));
      connect(this, SIGNAL(currentIndexChanged(int)), SLOT(indexChanged(int)));
      }

//---------------------------------------------------------
//   indexChanged
//---------------------------------------------------------

void MagBox::indexChanged(int idx)
      {
      emit magChanged(idx);
      }

//---------------------------------------------------------
//   txt2mag
//---------------------------------------------------------

double MagBox::getMag(ScoreView* canvas)
      {
      switch (MagIdx(currentIndex())) {
            case MagIdx::MAG_25:      return 0.25;
            case MagIdx::MAG_50:      return 0.5;
            case MagIdx::MAG_75:      return 0.75;
            case MagIdx::MAG_100:     return 1.0;
            case MagIdx::MAG_150:     return 1.5;
            case MagIdx::MAG_200:     return 2.0;
            case MagIdx::MAG_400:     return 4.0;
            case MagIdx::MAG_800:     return 8.0;
            case MagIdx::MAG_1600:    return 16.0;
            default:               break;
            }

      QSizeF s(canvas->fsize());
      double cw      = s.width();
      double ch      = s.height();
      double nmag    = canvas->mag();
      Score* score   = canvas->score();
      if (score == 0)
            return 1.0;
      const PageFormat* pf = score->pageFormat();
      switch (MagIdx(currentIndex())) {
            case MagIdx::MAG_PAGE_WIDTH:      // page width
                  nmag *= cw / (pf->width() * MScore::DPI);
                  break;
            case MagIdx::MAG_PAGE:     // page
                  {
                  double mag1 = cw  / (pf->width() * MScore::DPI);
                  double mag2 = ch / (pf->height() * MScore::DPI);
                  nmag  *= (mag1 > mag2) ? mag2 : mag1;
                  }
                  break;
            case MagIdx::MAG_DBL_PAGE:    // double page
                  {
                  double mag1 = cw / (pf->width()*2*MScore::DPI+50.0);
                  double mag2 = ch / (pf->height() * MScore::DPI);
                  nmag  *= (mag1 > mag2) ? mag2 : mag1;
                  }
                  break;
            case MagIdx::MAG_FREE:
                  return freeMag;
            default:
                  break;
            }
      if (nmag < 0.0001)
            nmag = canvas->mag();
      return nmag;
      }

//---------------------------------------------------------
//   MagValidator
//---------------------------------------------------------

MagValidator::MagValidator(QObject* parent)
   : QValidator(parent)
      {
      }

//---------------------------------------------------------
//   validate
//---------------------------------------------------------

QValidator::State MagValidator::validate(QString& input, int& /*pos*/) const
      {
      QComboBox* cb = (QComboBox*)parent();
      int mn = sizeof(magTable)/sizeof(*magTable);
      for (int i = 0; i < mn; ++i) {
            if (input == cb->itemText(i))
                  return QValidator::Acceptable;
            }
      QString d;
      for (int i = 0; i < input.size(); ++i) {
            QChar c = input[i];
            if (c.isDigit() || c == '.')
                  d.append(c);
            else if (c != '%')
                  return QValidator::Invalid;
            }
      if (d.isEmpty())
            return QValidator::Intermediate;
      bool ok;
      double nmag = d.toDouble(&ok);
      if (!ok)
            return QValidator::Invalid;
      if (nmag < 25.0 || nmag > 1600.0)
            return QValidator::Intermediate;
      return QValidator::Acceptable;
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void MagBox::setMag(double val)
      {
      const QSignalBlocker blocker(this);
      setCurrentIndex(int(MagIdx::MAG_FREE));
      freeMag = val;
      setItemText(int(MagIdx::MAG_FREE), QString("%1%").arg(freeMag * 100));
      }

//---------------------------------------------------------
//   setMagIdx
//---------------------------------------------------------

void MagBox::setMagIdx(MagIdx idx)
      {
      const QSignalBlocker blocker(this);
      setCurrentIndex(int(idx));
      }
}

