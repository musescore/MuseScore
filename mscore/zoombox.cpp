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
//   zoomEntries
//---------------------------------------------------------

const std::array<ZoomEntry, 13> zoomEntries { {
     {  ZoomIndex::ZOOM_25,           25, "25%"   },
     {  ZoomIndex::ZOOM_50,           50, "50%"   },
     {  ZoomIndex::ZOOM_75,           75, "75%"   },
     {  ZoomIndex::ZOOM_100,         100, "100%"  },
     {  ZoomIndex::ZOOM_150,         150, "150%"  },
     {  ZoomIndex::ZOOM_200,         200, "200%"  },
     {  ZoomIndex::ZOOM_400,         400, "400%"  },
     {  ZoomIndex::ZOOM_800,         800, "800%"  },
     {  ZoomIndex::ZOOM_1600,       1600, "1600%" },
     {  ZoomIndex::ZOOM_PAGE_WIDTH,    0, QT_TRANSLATE_NOOP("magTable", "Page Width") },
     {  ZoomIndex::ZOOM_WHOLE_PAGE,    0, QT_TRANSLATE_NOOP("magTable", "Whole Page") },
     {  ZoomIndex::ZOOM_TWO_PAGES,     0, QT_TRANSLATE_NOOP("magTable", "Two Pages") },
     {  ZoomIndex::ZOOM_FREE,          0, "" },
     } };

ZoomState ZoomBox::getDefaultLogicalZoom()
      {
      ZoomState result { ZoomIndex::ZOOM_100, 1.0 };

      // Convert the default-zoom preferences into a usable zoom index and logical zoom level.
      switch (static_cast<ZoomType>(preferences.getInt(PREF_UI_CANVAS_ZOOM_DEFAULT_TYPE))) {
            case ZoomType::PERCENTAGE: {
                  // Select a numeric preset zoom entry if the percentage corresponds to one; otherwise, select free zoom.
                  const auto logicalLevelPercentage = preferences.getInt(PREF_UI_CANVAS_ZOOM_DEFAULT_LEVEL);
                  const auto i = std::find(zoomEntries.cbegin(), zoomEntries.cend(), logicalLevelPercentage);
                  result.index = ((i != zoomEntries.cend()) && i->isNumericPreset()) ? i->index : ZoomIndex::ZOOM_FREE;
                  result.level = logicalLevelPercentage / 100.0;
                  }
                  break;
            case ZoomType::PAGE_WIDTH:
                  result.index = ZoomIndex::ZOOM_PAGE_WIDTH;
                  break;
            case ZoomType::WHOLE_PAGE:
                  result.index = ZoomIndex::ZOOM_WHOLE_PAGE;
                  break;
            case ZoomType::TWO_PAGES:
                  result.index = ZoomIndex::ZOOM_TWO_PAGES;
                  break;
            default:
                  Q_ASSERT(false);
                  break;
            }

      return result;
      }

//---------------------------------------------------------
//   ZoomBox
//---------------------------------------------------------

ZoomBox::ZoomBox(QWidget* parent)
   : QComboBox(parent)
   , _previousLogicalLevel(0.0)
   , _previousScoreView(nullptr)
      {
      setEditable(true);
      setInsertPolicy(QComboBox::InsertAtBottom);
      setToolTip(tr("Zoom"));
      setWhatsThis(tr("Zoom"));
      setAccessibleName(tr("Zoom"));
      setValidator(new ZoomValidator(this));
      setCompleter(nullptr);
      setFocusPolicy(Qt::StrongFocus);
      setFixedHeight(preferences.getInt(PREF_UI_THEME_ICONHEIGHT) + 8);  // hack
      setMaxCount(static_cast<int>(zoomEntries.size()));
      setMaxVisibleItems(static_cast<int>(zoomEntries.size()));
      for (const ZoomEntry& e : zoomEntries) {
            QString ts(QCoreApplication::translate("magTable", e.txt));
            addItem(ts, QVariant::fromValue(e.index));
            }
      resetToDefaultLogicalZoom();
      connect(this, SIGNAL(currentIndexChanged(int)), SLOT(indexChanged(int)));
      connect(lineEdit(), SIGNAL(returnPressed()), SLOT(textChanged()));
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void ZoomBox::textChanged()
      {
      if (!mscore->currentScoreView())
            return;

      const auto validation = ZoomValidator::validationHelper(currentText());

      if (std::get<0>(validation) == QValidator::Acceptable)
            setLogicalZoom(std::get<1>(validation), std::get<2>(validation) / 100.0);
      }

//---------------------------------------------------------
//   indexChanged
//---------------------------------------------------------

void ZoomBox::indexChanged(int index)
      {
      emit zoomChanged(itemData(index).value<ZoomIndex>(), _previousLogicalLevel);
      }

//---------------------------------------------------------
//   setLogicalZoom
//---------------------------------------------------------

void ZoomBox::setLogicalZoom(const ZoomIndex index, const qreal logicalLevel)
      {
      // Check of the zoom type has changed.
      if (static_cast<int>(index) != currentIndex()) {
            // Set the new zoom type, but don't emit any signals because that will be done below if needed.
            const QSignalBlocker blocker(this);
            setCurrentIndex(static_cast<int>(index));
            }

      // Check if either the logical zoom level has changed or the user has switched to a different score.
      if ((logicalLevel != _previousLogicalLevel) || (mscore && (mscore->currentScoreView() != _previousScoreView))) {
            if ((logicalLevel != _previousLogicalLevel)) {
                  // Convert the value to an integer percentage using half-to-even rounding (a.k.a. banker's rounding).
                  const auto logicalLevelPercentage = static_cast<int>((100.0 * logicalLevel) - std::remainder(100.0 * logicalLevel, 1.0));

                  qDebug("ZoomBox::setLogicalZoom(): Formatting logical zoom level as %d%% (rounded from %f)", logicalLevelPercentage, logicalLevel);
                  setItemText(static_cast<int>(ZoomIndex::ZOOM_FREE), QString("%1%").arg(logicalLevelPercentage));

                  _previousLogicalLevel = logicalLevel;
                  }

            if (mscore && (mscore->currentScoreView() != _previousScoreView))
                  _previousScoreView = mscore->currentScoreView();

            emit zoomChanged(static_cast<ZoomIndex>(currentIndex()), logicalLevel);
            }
      }

//---------------------------------------------------------
//   resetToDefaultLogicalZoom
//---------------------------------------------------------

void ZoomBox::resetToDefaultLogicalZoom()
      {
      const auto defaultZoom = getDefaultLogicalZoom();
      setLogicalZoom(defaultZoom.index, defaultZoom.level);
      }

//---------------------------------------------------------
//   ZoomValidator
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
      return std::get<0>(validationHelper(input));
      }

//---------------------------------------------------------
//   validationHelper
//---------------------------------------------------------

std::tuple<QValidator::State, ZoomIndex, int> ZoomValidator::validationHelper(const QString& input)
      {
      // Strip off the trailing '%', if any.
      const QString s = (input.right(1) == '%') ? input.left(input.length() - 1) : input;

      // Check if it's an empty string.
      if (s.isEmpty())
            return std::make_tuple(QValidator::Intermediate, ZoomIndex::ZOOM_FREE, 0);

      // Check if it's anything other than a valid integer.
      bool ok;
      const auto level = s.toInt(&ok);
      if (!ok)
            return std::make_tuple(QValidator::Invalid, ZoomIndex::ZOOM_FREE, 0);

      // Check if it's out of range.
      if ((level < 100.0 * ZOOM_LEVEL_MIN) || (level > 100.0 * ZOOM_LEVEL_MAX))
            return std::make_tuple(QValidator::Intermediate, ZoomIndex::ZOOM_FREE, level);

      // Check if it corresponds to one of the numeric presets.
      const auto i = std::find(zoomEntries.cbegin(), zoomEntries.cend(), level);
      if ((i != zoomEntries.cend()) && i->isNumericPreset())
            return std::make_tuple(QValidator::Acceptable, i->index, i->level);

      // Must be free zoom.
      return std::make_tuple(QValidator::Acceptable, ZoomIndex::ZOOM_FREE, level);
      }
}
