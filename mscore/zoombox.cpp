//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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
#include "zoombox.h"
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

struct ZoomEntry {
      const char* txt;
      ZoomIndex index;
      };

static constexpr ZoomEntry zoomEntries[] = {
     {  "25%",   ZoomIndex::ZOOM_25 },
     {  "50%",   ZoomIndex::ZOOM_50 },
     {  "75%",   ZoomIndex::ZOOM_75 },
     {  "100%",  ZoomIndex::ZOOM_100 },
     {  "150%",  ZoomIndex::ZOOM_150 },
     {  "200%",  ZoomIndex::ZOOM_200 },
     {  "400%",  ZoomIndex::ZOOM_400 },
     {  "800%",  ZoomIndex::ZOOM_800 },
     {  "1600%", ZoomIndex::ZOOM_1600 },
     {  QT_TRANSLATE_NOOP("magTable", "Page Width"), ZoomIndex::ZOOM_PAGE_WIDTH },
     {  QT_TRANSLATE_NOOP("magTable", "Whole Page"), ZoomIndex::ZOOM_WHOLE_PAGE },
     {  QT_TRANSLATE_NOOP("magTable", "Two Pages"),  ZoomIndex::ZOOM_TWO_PAGES },
     };

static constexpr ZoomIndex startZoomIndex = ZoomIndex::ZOOM_100;

//---------------------------------------------------------
//   MagBox
//---------------------------------------------------------

ZoomBox::ZoomBox(QWidget* parent)
   : QComboBox(parent)
      {
      _logicalLevel = 1.0;
      setEditable(true);
      setInsertPolicy(QComboBox::InsertAtBottom);
      setToolTip(tr("Zoom"));
      setWhatsThis(tr("Zoom"));
      setValidator(new ZoomValidator(this));
      setAutoCompletion(false);

      int i = 0;
      for (const ZoomEntry& e : zoomEntries) {
            QString ts(QCoreApplication::translate("magTable", e.txt));
            addItem(ts, QVariant::fromValue(e.index));
            if (e.index == startZoomIndex)
                  setCurrentIndex(i);
            ++i;
            }
      setMaxCount(i+1);
      addItem(QString("%1%").arg(_logicalLevel * 100), int(ZoomIndex::ZOOM_FREE));
      setFocusPolicy(Qt::StrongFocus);
      setAccessibleName(tr("Zoom"));
      setFixedHeight(preferences.getInt(PREF_UI_THEME_ICONHEIGHT) + 8);  // hack
      connect(this, SIGNAL(currentIndexChanged(int)), SLOT(indexChanged(int)));
      connect(lineEdit(), SIGNAL(returnPressed()), SLOT(textChanged()));
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void ZoomBox::textChanged()
      {
      if (!mscore->currentScoreView() || currentText().isEmpty())
            return;
      QString s = currentText();
      if (s.right(1) == "%")
            s = s.left(s.length()-1);

      bool ok;
      const qreal levelPercentage = s.toFloat(&ok);
      if (ok) {
            setLogicalZoomLevel(levelPercentage / 100.0);
            setCurrentIndex(static_cast<int>(ZoomIndex::ZOOM_FREE));
            }
      }

//---------------------------------------------------------
//   indexChanged
//---------------------------------------------------------

void ZoomBox::indexChanged(int index)
      {
      emit zoomIndexChanged(itemData(index).value<ZoomIndex>());
      }

//---------------------------------------------------------
//   getLogicalZoomLevel
//---------------------------------------------------------

qreal ZoomBox::getLogicalZoomLevel(ScoreView* canvas) const
      {
      qDebug() << "MagBox::getLogicalZoomLevel(): Returning: " << getPhysicalZoomLevel(canvas) / (mscore->physicalDotsPerInch() / DPI);
      return getPhysicalZoomLevel(canvas) / (mscore->physicalDotsPerInch() / DPI);
      }

//---------------------------------------------------------
//   getPhysicalZoomLevel
//---------------------------------------------------------

qreal ZoomBox::getPhysicalZoomLevel(ScoreView* canvas) const
      {
      Score* score   = canvas->score();
      if (score == 0)
            return 1.0;

      ZoomIndex idx           = ZoomIndex(currentIndex());
      qreal l2p            = mscore->physicalDotsPerInch() / DPI;
      double cw            = canvas->width();
      double ch            = canvas->height();
      qreal pw             = score->styleD(Sid::pageWidth);
      qreal ph             = score->styleD(Sid::pageHeight);
      double physicalLevel;

      switch (idx) {
            case ZoomIndex::ZOOM_25:      physicalLevel = 0.25 * l2p; break;
            case ZoomIndex::ZOOM_50:      physicalLevel = 0.5  * l2p; break;
            case ZoomIndex::ZOOM_75:      physicalLevel = 0.75 * l2p; break;
            case ZoomIndex::ZOOM_100:     physicalLevel = 1.0  * l2p; break;
            case ZoomIndex::ZOOM_150:     physicalLevel = 1.5  * l2p; break;
            case ZoomIndex::ZOOM_200:     physicalLevel = 2.0  * l2p; break;
            case ZoomIndex::ZOOM_400:     physicalLevel = 4.0  * l2p; break;
            case ZoomIndex::ZOOM_800:     physicalLevel = 8.0  * l2p; break;
            case ZoomIndex::ZOOM_1600:    physicalLevel = 16.0 * l2p; break;

            case ZoomIndex::ZOOM_PAGE_WIDTH:      // page width
                  physicalLevel = cw / (pw * DPI);
                  break;

            case ZoomIndex::ZOOM_WHOLE_PAGE:     // page
                  {
                  double mag1 = cw / (pw *  DPI);
                  double mag2 = ch / (ph * DPI);
                  physicalLevel  = (mag1 > mag2) ? mag2 : mag1;
                  }
                  break;

            case ZoomIndex::ZOOM_TWO_PAGES:    // double page
                  {
                  double mag1 = 0;
                  double mag2 = 0;
                  if (MScore::verticalOrientation()) {
                        mag1 = ch / (ph * 2 * DPI +  MScore::verticalPageGap);
                        mag2 = cw / (pw * DPI);
                        }
                  else {
                        mag1 = cw / (pw * 2 * DPI + 50);
                        mag2 = ch / (ph * DPI);
                        }
                  physicalLevel  = (mag1 > mag2) ? mag2 : mag1;
                  }
                  break;

            case ZoomIndex::ZOOM_FREE:
                  physicalLevel = _logicalLevel * l2p;
                  break;

            default:
                  physicalLevel = 0.0;
                  break;
            }
      if (physicalLevel < 0.0001)
            physicalLevel = canvas->physicalZoomLevel();

      return physicalLevel;
      }

//---------------------------------------------------------
//   MagValidator
//---------------------------------------------------------

ZoomValidator::ZoomValidator(QObject* parent)
   : QValidator(parent)
      {
      }

//---------------------------------------------------------
//   validate
//---------------------------------------------------------

QValidator::State ZoomValidator::validate(QString& input, int& /*pos*/) const
      {
      QComboBox* cb = (QComboBox*)parent();
      int mn = sizeof(zoomEntries)/sizeof(*zoomEntries);
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
      if (nmag < (100.0 * ZOOM_LEVEL_MIN) || nmag > (100.0 * ZOOM_LEVEL_MAX))
            return QValidator::Intermediate;
      return QValidator::Acceptable;
      }

//---------------------------------------------------------
//   setLogicalZoomLevel
//---------------------------------------------------------

void ZoomBox::setLogicalZoomLevel(const qreal logicalLevel)
      {
      _logicalLevel = logicalLevel;

      // Convert the value to an integer percentage using half-to-even rounding (a.k.a. banker's rounding).
      const auto logicalLevelPercentage = static_cast<int>((100.0 * logicalLevel) - std::remainder(100.0 * logicalLevel, 1.0));

      qDebug() << "MagBox::setLogicalZoomLevel(): Setting logical zoom level to: " << logicalLevelPercentage << " (rounded from " << logicalLevel << ")";
      setItemText(static_cast<int>(ZoomIndex::ZOOM_FREE), QString("%1%").arg(logicalLevelPercentage));
      }

//---------------------------------------------------------
//   setZoomIndex
//---------------------------------------------------------

void ZoomBox::setZoomIndex(ZoomIndex idx)
      {
      const QSignalBlocker blocker(this);
      setCurrentIndex(static_cast<int>(idx));
      }
}
