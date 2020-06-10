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
//    indices of items shown in QComboBox "ZoomBox"
//---------------------------------------------------------

enum class ZoomIndex : char {
       ZOOM_25, ZOOM_50, ZOOM_75, ZOOM_100, ZOOM_150, ZOOM_200, ZOOM_400, ZOOM_800, ZOOM_1600,
       ZOOM_PAGE_WIDTH, ZOOM_WHOLE_PAGE, ZOOM_TWO_PAGES,
       ZOOM_FREE
      };

//---------------------------------------------------------
//   ZoomValidator
//---------------------------------------------------------

class ZoomValidator : public QValidator {
      Q_OBJECT

      virtual State validate(QString&, int&) const;

   public:
      ZoomValidator(QObject* parent = 0);
      };

//---------------------------------------------------------
//   ZoomBox
//---------------------------------------------------------

class ZoomBox : public QComboBox {
      Q_OBJECT

      qreal _logicalLevel;

   private slots:
      void indexChanged(int);
      void textChanged();

   signals:
      void zoomIndexChanged(ZoomIndex);

   public:
      ZoomBox(QWidget* parent = 0);
      void setLogicalZoomLevel(qreal);
      void setZoomIndex(ZoomIndex);
      qreal getPhysicalZoomLevel(ScoreView*) const;
      qreal getLogicalZoomLevel(ScoreView*) const;
      void setEnabled(bool val) { QComboBox::setEnabled(val); }
      QString currentText() const { return QComboBox::currentText(); }
      int count() const { return QComboBox::count(); }
      void removeItem(int i) { QComboBox::removeItem(i); }
      };


} // namespace Ms

Q_DECLARE_METATYPE(Ms::ZoomIndex);

#endif



