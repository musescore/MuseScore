//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

namespace Ms {

//---------------------------------------------------------
//   MsSplashScreen
//---------------------------------------------------------

class MsSplashScreen : public QSplashScreen {
      static const QSize designSize;

      static const QRectF designDevBuildIconRect;
      static const QRectF designDevBuildTextRect;
      static const QRectF designLogotypeRect;
      static const QRectF designMessageTextRect;
      static const QRectF designMiscTextRect;

      static const double gradientDitherAmount;

      static const std::tuple<QColor, QColor> stableBuildGradientColors;
      static const std::tuple<QColor, QColor> unstableBuildGradientColors;

      static const QColor textColor;

      const QImage _bgImage;
      QSvgRenderer _devBuildIconRenderer;
      QSvgRenderer _logotypeRenderer;
      QString _miscText;

      const QRectF _devBuildIconRect;
      const QRectF _devBuildTextRect;
      QRectF _logotypeRect;
      const QRectF _messageTextRect;
      const QRectF _miscTextRect;

      QRectF scaleRect(const QRectF& designRect) const;
      QRectF scaleSvgRect(const QRectF& designRect, const QSvgRenderer& renderer) const;
      qreal scaleX(const qreal designX) const;
      qreal scaleY(const qreal designY) const;

      static QImage createBackgroundImage(const int w, const int h, const std::tuple<QColor, QColor>& gradientColors);
      static void drawVerticalGradient(QImage& image, const std::tuple<QColor, QColor>& colors, const double ditherAmount);
      static void drawDebugRect(QPainter* const painter, const QRectF& rect, const int r = 0x80, const int g = 0x80, const int b = 0x80, const int a = 0x80);

public:
      MsSplashScreen();
      void drawContents(QPainter* painter) override;
      };

}
