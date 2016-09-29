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

#include "preferences.h"
#include "magbox.h"
#include "scoreview.h"
#include "libmscore/page.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/mscore.h"

namespace Ms {

//---------------------------------------------------------
//   magTable
//    list of strings shown in QComboBox "MagBox"
//---------------------------------------------------------

struct MagEntry {
      const char* txt;
      MagIdx idx;
      };

static MagIdx startMag = MagIdx::MAG_100;

static const MagEntry magTable[] = {
     {  "25%",   MagIdx::MAG_25 },
     {  "50%",   MagIdx::MAG_50 },
     {  "75%",   MagIdx::MAG_75 },
     {  "100%",  MagIdx::MAG_100 },
     {  "150%",  MagIdx::MAG_150 },
     {  "200%",  MagIdx::MAG_200 },
     {  "400%",  MagIdx::MAG_400 },
     {  "800%",  MagIdx::MAG_800 },
     {  "1600%", MagIdx::MAG_1600 },
     {  QT_TRANSLATE_NOOP("magTable","Page Width"), MagIdx::MAG_PAGE_WIDTH },
     {  QT_TRANSLATE_NOOP("magTable","Whole Page"), MagIdx::MAG_PAGE },
     {  QT_TRANSLATE_NOOP("magTable","Two Pages"),  MagIdx::MAG_DBL_PAGE },
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

      int i = 0;
      for (const MagEntry& e : magTable) {
            QString ts(QCoreApplication::translate("magTable", e.txt));
            addItem(ts, QVariant::fromValue(e.idx));
            if (e.idx == startMag)
                  setCurrentIndex(i);
            ++i;
            }
      setMaxCount(i+1);
      addItem(QString("%1%").arg(freeMag * 100), int(MagIdx::MAG_FREE));
      setFocusPolicy(Qt::StrongFocus);
      setAccessibleName(tr("Zoom"));
      setFixedHeight(preferences.iconHeight + 8);  // hack
      connect(this, SIGNAL(currentIndexChanged(int)), SLOT(indexChanged(int)));
      connect(lineEdit(), SIGNAL(returnPressed()), SLOT(textChanged()));
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void MagBox::textChanged()
      {
      if (!mscore->currentScoreView() || currentText().isEmpty())
            return;
      QString s = currentText();
      if (s.right(1) == "%")
            s = s.left(s.length()-1);

      bool ok;
      qreal magVal = s.toFloat(&ok);
      if (ok) {
            setMag((double)(magVal/100.0));
            emit magChanged(MagIdx::MAG_FREE);
            }
      }

//---------------------------------------------------------
//   indexChanged
//---------------------------------------------------------

void MagBox::indexChanged(int idx)
      {
      emit magChanged(itemData(idx).value<MagIdx>());
      }

//---------------------------------------------------------
//   getLMag
//    get logical scale
//---------------------------------------------------------

double MagBox::getLMag(ScoreView* canvas) const
      {
      return getMag(canvas) / (mscore->physicalDotsPerInch() / DPI);
      }

//---------------------------------------------------------
//   getMag
//    get physical scale
//---------------------------------------------------------

double MagBox::getMag(ScoreView* canvas) const
      {
      Score* score   = canvas->score();
      if (score == 0)
            return 1.0;

      MagIdx idx           = MagIdx(currentIndex());
      qreal pmag           = mscore->physicalDotsPerInch() / DPI;
      double cw            = canvas->width();
      double ch            = canvas->height();
      const PageFormat* pf = score->pageFormat();
      double nmag;

      switch (idx) {
            case MagIdx::MAG_25:      nmag = 0.25 * pmag; break;
            case MagIdx::MAG_50:      nmag = 0.5  * pmag; break;
            case MagIdx::MAG_75:      nmag = 0.75 * pmag; break;
            case MagIdx::MAG_100:     nmag = 1.0  * pmag; break;
            case MagIdx::MAG_150:     nmag = 1.5  * pmag; break;
            case MagIdx::MAG_200:     nmag = 2.0  * pmag; break;
            case MagIdx::MAG_400:     nmag = 4.0  * pmag; break;
            case MagIdx::MAG_800:     nmag = 8.0  * pmag; break;
            case MagIdx::MAG_1600:    nmag = 16.0 * pmag; break;

            case MagIdx::MAG_PAGE_WIDTH:      // page width
                  nmag = cw / (pf->width() * DPI);
                  break;

            case MagIdx::MAG_PAGE:     // page
                  {
                  double mag1 = cw / (pf->width() *  DPI);
                  double mag2 = ch / (pf->height() * DPI);
                  nmag  = (mag1 > mag2) ? mag2 : mag1;
                  }
                  break;

            case MagIdx::MAG_DBL_PAGE:    // double page
                  {
                  double mag1 = 0;
                  double mag2 = 0;
                  if (MScore::verticalOrientation()) {
                        mag1 = ch / (pf->height() * 2 * DPI +  MScore::verticalPageGap);
                        mag2 = cw / (pf->width() * DPI);
                        }
                  else {
                        mag1 = cw / (pf->width() * 2 * DPI + 50);
                        mag2 = ch / (pf->height() * DPI);
                        }
                  nmag  = (mag1 > mag2) ? mag2 : mag1;
                  }
                  break;

            case MagIdx::MAG_FREE:
                  nmag = freeMag * pmag;
                  break;

            default:
                  nmag = 0.0;
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

