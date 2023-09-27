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

#include "mssplashscreen.h"

#include "musescore.h"
#include "libmscore/fraction.h"
#include "config.h"

#include <random>

#include <QDate>

namespace Ms {

const QSize MsSplashScreen::designSize { 720, 405 };

const QRectF MsSplashScreen::designDevBuildIconRect { 25.0,  51.0, 670.0, 38.0 };
const QRectF MsSplashScreen::designDevBuildTextRect { 25.0,  98.0, 670.0, 46.0 };
const QRectF MsSplashScreen::designLogotypeRect     { 25.0, 153.0, 670.0, 74.0 };
const QRectF MsSplashScreen::designMessageTextRect  { 25.0, 270.0, 670.0, 56.0 };
const QRectF MsSplashScreen::designMiscTextRect     { 25.0, 326.0, 670.0, 56.0 };

const double MsSplashScreen::gradientDitherAmount { 0.25 };

const std::tuple<QColor, QColor> MsSplashScreen::stableBuildGradientColors   { "#0885DD", "#0C70B6" };
const std::tuple<QColor, QColor> MsSplashScreen::unstableBuildGradientColors { "#04426E", "#063759" };

const QColor MsSplashScreen::textColor { 0xCCFFFFFF };

//---------------------------------------------------------
//   MsSplashScreen constructor
//---------------------------------------------------------

MsSplashScreen::MsSplashScreen()
   : QSplashScreen(QPixmap(designSize))
   , _bgImage(createBackgroundImage(width(), height(), MuseScore::unstable() ? unstableBuildGradientColors : stableBuildGradientColors))
   , _devBuildIconRenderer(QString(":/data/maintenance.svg"), this)
   , _miscText(QString(tr("Version %1")).arg(VERSION) + "\nwww.musescore.org")

   , _devBuildIconRect(scaleSvgRect(designDevBuildIconRect, _devBuildIconRenderer))
   , _devBuildTextRect(scaleRect(designDevBuildTextRect))
   , _messageTextRect(scaleRect(designMessageTextRect))
   , _miscTextRect(scaleRect(designMiscTextRect))
      {
      setWindowTitle("MuseScore Startup");

      QDate d = QDate::currentDate();
      if (d.day() >= 24 && d.day() <=27 && d.month() == 12) {
            _logotypeRenderer.load(QString(":/data/musescore-logotype-c.svg"));
      }
      else {
            _logotypeRenderer.load(QString(":/data/musescore-logotype.svg"));
      }

      _logotypeRect = scaleSvgRect(designLogotypeRect, _logotypeRenderer);


#ifdef Q_OS_MAC
      // To have session dialog on top of splash screen on Mac.
      setWindowFlags(Qt::FramelessWindowHint);
#endif
      }

//---------------------------------------------------------
//   drawContents
//---------------------------------------------------------

void MsSplashScreen::drawContents(QPainter* painter)
      {
      painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

      // Background gradient.
      painter->drawImage(0, 0, _bgImage);

      // Development build icon and text.
      /*if (MuseScore::unstable())*/ {
            _devBuildIconRenderer.render(painter, _devBuildIconRect);
            drawDebugRect(painter, _devBuildIconRect, 0xFF, 0x80, 0x80);

            painter->setPen(textColor);
            painter->drawText(_devBuildTextRect, Qt::AlignHCenter | Qt::AlignTop, tr("Evolution"));
            drawDebugRect(painter, _devBuildTextRect, 0xFF, 0x80, 0x80);
            }

      // MuseScore logotype.
      _logotypeRenderer.render(painter, _logotypeRect);
      drawDebugRect(painter, _logotypeRect, 0xFF, 0x80, 0x80);

      // Status text.
      painter->setPen(textColor);
      painter->drawText(_messageTextRect, Qt::AlignHCenter | Qt::AlignTop, message());
      drawDebugRect(painter, _messageTextRect, 0x80, 0xFF, 0x80);

      // Miscellaneous text (version information and Web site).
      painter->drawText(_miscTextRect, Qt::AlignRight | Qt::AlignBottom, _miscText);
#if defined(WIN_PORTABLE)
      // Additional text for Windows Portable version.
      painter->setPen(textColor);
      painter->drawText(_miscTextRect, Qt::AlignLeft | Qt::AlignBottom, tr("Portable version") + " (portableapps.com)");
#endif
      drawDebugRect(painter, _miscTextRect, 0x80, 0x80, 0xFF);
      }

//---------------------------------------------------------
//   scaleX
//---------------------------------------------------------

qreal MsSplashScreen::scaleX(const qreal designX) const
      {
      return designX * width() / designSize.width();
      }

//---------------------------------------------------------
//   scaleY
//---------------------------------------------------------

qreal MsSplashScreen::scaleY(const qreal designY) const
      {
      return designY * height() / designSize.height();
      }

//---------------------------------------------------------
//   scaleRect
//---------------------------------------------------------

QRectF MsSplashScreen::scaleRect(const QRectF& designRect) const
      {
      return QRectF(scaleX(designRect.x()), scaleY(designRect.y()), scaleX(designRect.width()), scaleY(designRect.height()));
      }

//---------------------------------------------------------
//   scaleSvgRect
//---------------------------------------------------------

QRectF MsSplashScreen::scaleSvgRect(const QRectF& designRect, const QSvgRenderer& renderer) const
      {
      // Maximize the height of the SVG asset within the available height while maintaining the original aspect ratio.
      const auto svgViewBox = renderer.viewBoxF();
      const auto arCorrectedDesignWidth = designRect.height() * svgViewBox.width() / svgViewBox.height();

      return scaleRect(QRectF(designRect.x() + (designRect.width() - arCorrectedDesignWidth) / 2, designRect.y(), arCorrectedDesignWidth, designRect.height()));
      }

//---------------------------------------------------------
//   createBackgroundImage
//---------------------------------------------------------

QImage MsSplashScreen::createBackgroundImage(const int w, const int h, const std::tuple<QColor, QColor>& gradientColors)
      {
      QImage result(w, h, QImage::Format_ARGB32);

      // First, create an image containing a vertical gradient (top to bottom). This image will later be rotated to the correct
      // angle so that the gradient runs from the top-left to the bottom-right corner. The final gradient needs fill the entire
      // image without getting truncated, so we calculate the minimum size of the rectangle that is needed to enclose the
      // background image after being rotated to the required angle. We round up the final size to the nearest pixel to ensure that
      // we don't cut any corners (literally).
      const auto gradientAngle = std::atan2(h, w);
      const auto gradientImageWidth = static_cast<int>(std::ceil(h * std::sin(gradientAngle) + w * std::cos(gradientAngle)));
      const auto gradientImageHeight = static_cast<int>(std::ceil(2.0 * w * std::sin(gradientAngle)));
      QImage gradientImage(gradientImageWidth, gradientImageHeight, QImage::Format_ARGB32);
      drawVerticalGradient(gradientImage, gradientColors, gradientDitherAmount);

      // Now that we have the gradient image, transfer it to the background image, rotating it around the center.
      QPainter painter(&result);
      painter.setRenderHints(QPainter::SmoothPixmapTransform);
      painter.translate(0.5 * w, 0.5 * h);
      painter.rotate(-qRadiansToDegrees(gradientAngle));
      painter.translate(-0.5 * w, -0.5 * h);
      painter.drawImage((w - gradientImageWidth) / 2, (h - gradientImageHeight) / 2, gradientImage);

      return result;
      }

//---------------------------------------------------------
//   drawVerticalGradient
//---------------------------------------------------------

void MsSplashScreen::drawVerticalGradient(QImage& image, const std::tuple<QColor, QColor>& colors, const double ditherAmount)
      {
      // Although we could ask Qt to render our gradient for us, we don't, because Qt produces unsightly banding artifacts when
      // rendering gradients of colors that are very similar to each other. This happens because 24 bits of color just aren't
      // enough to render a smooth gradient in those cases. So instead, we render the gradient ourselves, calculating everything in
      // floating point and then dithering. This randomly distributes the inevitable rounding errors such that the average value of
      // all the pixels in each scan line ends up as close as possible to the correct floating-point value.
      //
      // Dithering eliminates the banding but introduces some visual noise. For that reason, the caller gets to specify the amount
      // of dither to be applied (i.e., the standard deviation of the error-distribution function, expressed in pixel values from 0
      // to 255). It's best to specify the smallest amount of dither that makes the banding unnoticeable.
      //
      // Note: We interpolate over the HSV color space, which produces much better results than RGB and is also convenient to use
      // because QColor supports it natively. It would be even better to use a perceptually uniform color space such as CIELUV, but
      // that would be both more complicated and more computationally expensive. Fortunately, for our purposes (since we happen to
      // be rendering gradients between colors with very similar hues), HSV is more than sufficient.
      std::default_random_engine rng;

      const auto& topColor = std::get<0>(colors);
      const auto& bottomColor = std::get<1>(colors);

      for (int y = 0; y < image.height(); y++) {
            const auto progress = static_cast<double>(y) / (image.height() - 1);

            const auto h = topColor.hsvHueF() + (bottomColor.hsvHueF() - topColor.hsvHueF() ) * progress;
            const auto s = topColor.hsvSaturationF() + (bottomColor.hsvSaturationF() - topColor.hsvSaturationF()) * progress;
            const auto v = topColor.valueF() + (bottomColor.valueF() - topColor.valueF() ) * progress;

            const auto color(QColor::fromHsvF(h, s, v));

            const auto r = color.redF() * 0xFF;
            const auto g = color.greenF() * 0xFF;
            const auto b = color.blueF() * 0xFF;

            const auto rRounded = std::round(r + 0.5);
            const auto gRounded = std::round(g + 0.5);
            const auto bRounded = std::round(b + 0.5);

            std::normal_distribution<> rDist(r - rRounded, ditherAmount);
            std::normal_distribution<> gDist(g - gRounded, ditherAmount);
            std::normal_distribution<> bDist(b - bRounded, ditherAmount);

            const auto scanLine = reinterpret_cast<QRgb*>(image.scanLine(y));
            for (int x = 0; x < image.width(); x++)
                  scanLine[x] = qRgb(
                     qBound(0x00, static_cast<int>(rRounded + rDist(rng)), 0xFF),
                     qBound(0x00, static_cast<int>(gRounded + gDist(rng)), 0xFF),
                     qBound(0x00, static_cast<int>(bRounded + bDist(rng)), 0xFF)
                     );
            }
      }

//---------------------------------------------------------
//   drawDebugRect
//---------------------------------------------------------

void MsSplashScreen::drawDebugRect(QPainter* const painter, const QRectF& rect, const int r, const int g, const int b, const int a)
      {
// Set this to 1 to help visualize the layout.
#if 0
      painter->fillRect(rect, QColor(r, g, b, a));
#else
      Q_UNUSED(painter);
      Q_UNUSED(rect);
      Q_UNUSED(r);
      Q_UNUSED(g);
      Q_UNUSED(b);
      Q_UNUSED(a);
#endif
      }

}
