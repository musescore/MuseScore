//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#ifndef __SVGRENDERER_H__
#define __SVGRENDERER_H__

namespace Ms {

//---------------------------------------------------------------------------
//   SvgRenderer - Create QPixmaps from SVGs.
//
// Unlike simply calling QPixmap("/path/to/image.svg"), this class creates
// pixmaps that are appropriately scaled for HDPI/retina displays.
//
// Note: Do you really need a pixmap? Instead of adding a pixmap to a QLabel,
// consider replacing the QLabel with a QSvgWidget instead as this avoids
// rasterising the vector image unnecessarily. Pixmaps are only useful for
// legacy Qt classes that require pixmaps, such as QSplashScreen & QWizard.
//---------------------------------------------------------------------------

class SvgRenderer : public QSvgRenderer
      {
   Q_OBJECT

   public:
      SvgRenderer(const QString &filename, QObject *parent = nullptr) : QSvgRenderer(filename, parent) {}
      QPixmap hdpiPixmap(QSize size = QSize(), bool scaleWithGui = false, const QColor &background = Qt::transparent);
      };

}
#endif // __SVGRENDERER_H__
