//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef ZOOMBOX_H
#define ZOOMBOX_H

namespace Ms {

class ScoreView;

//---------------------------------------------------------
//   ZoomIndex
//    Indices of the items in the zoom box.
//---------------------------------------------------------

enum class ZoomIndex : char {
       ZOOM_25, ZOOM_50, ZOOM_75, ZOOM_100, ZOOM_150, ZOOM_200, ZOOM_400, ZOOM_800, ZOOM_1600,
       ZOOM_PAGE_WIDTH, ZOOM_WHOLE_PAGE, ZOOM_TWO_PAGES,
       ZOOM_FREE
      };

//---------------------------------------------------------
//   zoomEntry
//    The string, index, and zoom level for each item in the zoom box.
//---------------------------------------------------------

struct ZoomEntry {
      ZoomIndex index;
      int level; // Must be set to 0 for all entries that aren't numeric presets (including ZOOM_FREE).
      const char* txt;

      bool isNumericPreset() const { return level != 0; }

      friend bool operator==(const ZoomEntry& e, const int l) { return e.level == l; }
      friend bool operator==(const ZoomEntry& e, const ZoomIndex i) { return e.index == i; }
      };

//---------------------------------------------------------
//   ZoomState
//    The zoom index and level of a single zoom state.
//---------------------------------------------------------

struct ZoomState {
      ZoomIndex index;
      qreal level;

      friend bool operator!=(const ZoomState& l, const ZoomState& r) { return (l.index != r.index) || (l.level != r.level); }
      };

//---------------------------------------------------------
//   zoomEntries
//    All of the entries in the zoom box.
//---------------------------------------------------------

extern const std::array<ZoomEntry, 13> zoomEntries;

//---------------------------------------------------------
//   ZoomValidator
//---------------------------------------------------------

class ZoomValidator : public QValidator {
      Q_OBJECT

      virtual State validate(QString&, int&) const;

   public:
      ZoomValidator(QObject* parent = 0);

      static std::tuple<QValidator::State, ZoomIndex, int> validationHelper(const QString& input);
      };

//---------------------------------------------------------
//   ZoomBox
//---------------------------------------------------------

class ZoomBox : public QComboBox {
      Q_OBJECT

      qreal _previousLogicalLevel;
      ScoreView* _previousScoreView;

   private slots:
      void indexChanged(int);
      void textChanged();

   signals:
      void zoomChanged(const ZoomIndex zoomIndex, const qreal logicalFreeZoomLevel = 0.0);

   public:
      ZoomBox(QWidget* parent = 0);

      static ZoomState getDefaultLogicalZoom();

      void setLogicalZoom(const ZoomIndex index, const qreal logicalLevel);
      void resetToDefaultLogicalZoom();
      void setEnabled(bool val) { QComboBox::setEnabled(val); }
      QString currentText() const { return QComboBox::currentText(); }
      int count() const { return QComboBox::count(); }
      void removeItem(int i) { QComboBox::removeItem(i); }
      };


} // namespace Ms

Q_DECLARE_METATYPE(Ms::ZoomIndex);

#endif
