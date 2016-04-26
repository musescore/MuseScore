//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "stylehelper.h"
#include "colorscheme.h"
#include "colorutils.h"

const qreal StyleHelper::_slabThickness = 0.45;
const qreal StyleHelper::_shadowGain    = 1.5;
const qreal StyleHelper::_glowBias      = 0.6;

//---------------------------------------------------------
//   StyleHelper
//---------------------------------------------------------

StyleHelper::StyleHelper() {
      _contrast = .7;
      _bgcontrast = qMin(1.0, 0.9 * _contrast / .7);

      m_dockFrameCache.setMaxCost(1);
      m_scrollHoleCache.setMaxCost(10);
      m_backgroundCache.setMaxCost(64);
      m_windecoButtonCache.setMaxCost(64);
      m_windecoButtonGlowCache.setMaxCost(64);
      }

//---------------------------------------------------------
//   reloadConfig
//---------------------------------------------------------

void StyleHelper::reloadConfig() {
      _bgcontrast = qMin(1.0, 0.9 * _contrast / 0.7);

      _viewFocusBrush = StatefulBrush(ColorScheme::View, ColorScheme::FocusColor);
      _viewHoverBrush = StatefulBrush(ColorScheme::View, ColorScheme::HoverColor);
      _viewNegativeTextBrush = StatefulBrush(ColorScheme::View, ColorScheme::NegativeText);
      }

//---------------------------------------------------------
//   invalidateCaches
//---------------------------------------------------------

void StyleHelper::invalidateCaches() {
      m_dialSlabCache.clear();
      m_roundSlabCache.clear();
      m_holeFocusedCache.clear();
      m_midColorCache.clear();
      m_progressBarCache.clear();
      m_cornerCache.clear();
      m_selectionCache.clear();
      m_slabSunkenCache.clear();
      m_slabInvertedCache.clear();
      m_holeCache.clear();
      m_holeFlatCache.clear();
      m_slopeCache.clear();
      m_grooveCache.clear();
      m_slitCache.clear();
      m_dockFrameCache.clear();
      m_scrollHoleCache.clear();

      m_slabCache.clear();
      m_decoColorCache.clear();
      m_lightColorCache.clear();
      m_darkColorCache.clear();
      m_shadowColorCache.clear();
      m_backgroundTopColorCache.clear();
      m_backgroundBottomColorCache.clear();
      m_backgroundRadialColorCache.clear();
      m_backgroundColorCache.clear();
      m_backgroundCache.clear();
      m_dotCache.clear();
      m_windecoButtonCache.clear();
      m_windecoButtonGlowCache.clear();
      }

//---------------------------------------------------------
//   setMaxCacheSize
//---------------------------------------------------------

void StyleHelper::setMaxCacheSize( int value ) {
      m_windecoButtonCache.setMaxCost( value );
      m_windecoButtonGlowCache.setMaxCost( value );
      m_slabCache.setMaxCacheSize( value );
      m_backgroundCache.setMaxCost( value );
      m_dotCache.setMaxCost( value );

      m_dialSlabCache.setMaxCacheSize( value );
      m_roundSlabCache.setMaxCacheSize( value );
      m_holeFocusedCache.setMaxCacheSize( value );

      m_progressBarCache.setMaxCost( value );
      m_cornerCache.setMaxCost( value );
      m_selectionCache.setMaxCost( value );
      m_slabSunkenCache.setMaxCost( value );
      m_slabInvertedCache.setMaxCost( value );
      m_holeCache.setMaxCost( value );
      m_holeFlatCache.setMaxCost( value );
      m_slopeCache.setMaxCost( value );
      m_grooveCache.setMaxCost( value );
      m_slitCache.setMaxCost( value );
      m_dockFrameCache.setMaxCost( value );
      m_scrollHoleCache.setMaxCost( value );
      }

//---------------------------------------------------------
//   checkAutoFillBackground
//---------------------------------------------------------

const QWidget* StyleHelper::checkAutoFillBackground(const QWidget* w) const {
      if (!w)
            return 0;
      if (w->autoFillBackground())
            return w;
      if (w->isWindow())
            return 0;
      for (const QWidget* parent = w->parentWidget(); parent; parent = parent->parentWidget()) {
            if (parent->autoFillBackground())
                  return parent;
            if (parent == w->window())
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   backgroundColor
//---------------------------------------------------------

const QColor& StyleHelper::backgroundColor(const QColor& color, qreal ratio) const {
      const quint64 key((quint64(color.rgba()) << 32) | int(ratio * 512));
      QColor* out = m_backgroundColorCache.object(key);
      if (!out) {
            if (ratio < 0.5) {
                  const qreal a(2.0 * ratio);
                  out = new QColor(ColorUtils::mix(backgroundTopColor(color), color, a));
                  }
            else {
                  const qreal a( 2.0 * ratio - 1 );
                  out = new QColor(ColorUtils::mix(color, backgroundBottomColor(color), a));
                  }
            m_backgroundColorCache.insert(key, out);
            }
      return *out;
      }

//---------------------------------------------------------
//   backgroundTopColor
//---------------------------------------------------------

const QColor& StyleHelper::backgroundTopColor(const QColor& color) const {
      const quint64 key(color.rgba());
      QColor* out(m_backgroundTopColorCache.object(key));
      if (!out) {
            if (lowThreshold(color) )
                  out = new QColor(ColorScheme::shade(color, ColorScheme::MidlightShade, 0.0) );
            else {
                  const qreal my(ColorUtils::luma(ColorScheme::shade(color, ColorScheme::LightShade, 0.0) ) );
                  const qreal by(ColorUtils::luma(color) );
                  out = new QColor(ColorUtils::shade(color, (my - by) * _bgcontrast) );
                  }
            m_backgroundTopColorCache.insert( key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   backgroundBottomColor
//---------------------------------------------------------

const QColor& StyleHelper::backgroundBottomColor(const QColor& color) const {
      const quint64 key( color.rgba() );
      QColor* out(m_backgroundBottomColorCache.object(key));
      if (!out) {
            const QColor midColor(ColorScheme::shade(color, ColorScheme::MidShade, 0.0));
            if (lowThreshold(color))
                  out = new QColor(midColor);
            else {
                  const qreal by(ColorUtils::luma(color) );
                  const qreal my(ColorUtils::luma(midColor) );
                  out = new QColor(ColorUtils::shade(color, (my - by) * _bgcontrast) );
                  }
            m_backgroundBottomColorCache.insert( key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   lowThreshold
//---------------------------------------------------------

bool StyleHelper::lowThreshold(const QColor& color) const {
      const quint32 key( color.rgba() );
      ColorMap::iterator iter( m_lowThreshold.find( key ) );
      if (iter != m_lowThreshold.end())
            return iter.value();
      else {
            const QColor darker(ColorScheme::shade(color, ColorScheme::MidShade, 0.5 ) );
            const bool result(ColorUtils::luma(darker) > ColorUtils::luma(color) );
            m_lowThreshold.insert(key, result);
            return result;
            }
      }

//---------------------------------------------------------
//   highThreshold
//---------------------------------------------------------

bool StyleHelper::highThreshold(const QColor& color) const {
      const quint32 key( color.rgba() );
      ColorMap::iterator iter( m_highThreshold.find( key ) );
      if (iter != m_highThreshold.end() )
            return iter.value();
      else {
            const QColor lighter(ColorScheme::shade(color, ColorScheme::LightShade, 0.5 ) );
            const bool result(ColorUtils::luma(lighter) < ColorUtils::luma(color) );
            m_highThreshold.insert(key, result);
            return result;
            }
      }

//---------------------------------------------------------
//   hole
//---------------------------------------------------------

TileSet* StyleHelper::hole(const QColor& color, qreal shade, int size, bool outline) const {
      const quint64 key( (quint64(color.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size << 1 | outline );
      TileSet* tileSet = m_holeCache.object(key);

      if (!tileSet) {
            const int rsize( (int)ceil(qreal(size) * 5.0 / 7.0 ) );
            QImage image(rsize * 2, rsize * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(2, 2, 10, 10);

            // hole mask
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setBrush(Qt::black);
            p.drawEllipse(3, 3, 8, 8);

            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
            if ( outline ) {
                  QLinearGradient blend( 0, 3, 0, 11 );
                  blend.setColorAt(0, Qt::transparent );
                  blend.setColorAt(1, calcDarkColor( color ) );

                  p.setBrush( Qt::NoBrush );
                  p.setPen( QPen( blend, 1 ) );
                  p.drawEllipse( QPointF(3, 3.5), 8, 7 );
                  p.setPen( Qt::NoPen );
                  }

            // shadow
            drawInverseShadow(p, calcShadowColor( color ), 3, 8, 0.0);

            p.end();

            tileSet = new TileSet(QPixmap::fromImage(image), rsize, rsize, rsize, rsize, rsize - 1, rsize, 2, 1);

            m_holeCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   holeFlat
//---------------------------------------------------------

TileSet* StyleHelper::holeFlat(const QColor& color, qreal shade, int size) const {
      const quint64 key((quint64(color.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size );
      TileSet* tileSet = m_holeFlatCache.object(key);

      if (!tileSet) {
            const int rsize((int)ceil(qreal(size) * 5.0 / 7.0) );
            QImage image(rsize * 2, rsize * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(2, 2, 10, 10);

            // hole
            drawHole(p, color, shade, 7);

            // hole inside
            p.setBrush(color);
            p.drawEllipse(QRectF(3.4, 3.4, 7.2, 7.2));

            p.end();

            tileSet = new TileSet(QPixmap::fromImage(image), rsize, rsize, rsize, rsize, rsize - 1, rsize, 2, 1);
            m_holeFlatCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   drawHole
//---------------------------------------------------------

void StyleHelper::drawHole(QPainter& p, const QColor& color, qreal shade, int r) const {
      const int r2( 2 * r );
      const QColor base(ColorUtils::shade(color, shade) );
      const QColor light(ColorUtils::shade(calcLightColor(color), shade) );
      const QColor dark(ColorUtils::shade(calcDarkColor(color), shade) );
      const QColor mid(ColorUtils::shade(calcMidColor(color), shade) );

      // bevel
      const qreal y(ColorUtils::luma(base) );
      const qreal yl(ColorUtils::luma(light) );
      const qreal yd(ColorUtils::luma(dark) );
      QLinearGradient bevelGradient1(0, 2, 0, r2 - 2);
      bevelGradient1.setColorAt(0.2, dark);
      bevelGradient1.setColorAt(0.5, mid);
      bevelGradient1.setColorAt(1.0, light);
      if (y < yl && y > yd) {
            // no middle when color is very light/dark
            bevelGradient1.setColorAt(0.6, base);
            }
      p.setBrush(bevelGradient1);
      p.drawEllipse(3, 3, r2 - 6, r2 - 6);

      // mask
      QRadialGradient maskGradient(r, r, r - 2);
      maskGradient.setColorAt(0.80, Qt::black );
      maskGradient.setColorAt(0.90, alphaColor( Qt::black, 0.55) );
      maskGradient.setColorAt(1.00, Qt::transparent );
      p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
      p.setBrush(maskGradient);
      p.drawRect(0, 0, r2, r2);
      p.setCompositionMode(QPainter::CompositionMode_SourceOver);
      }

//---------------------------------------------------------
//   fillHole
//---------------------------------------------------------

void StyleHelper::fillHole(QPainter& p, const QRect& rect, int size) const {
      const qreal s( (3.0 * size) / 7.0 );
      p.drawRoundedRect(rect.adjusted(s, s, -s, -s), 4, 4);
      }

//---------------------------------------------------------
//   renderHole
//---------------------------------------------------------

void StyleHelper::renderHole(QPainter* p, const QColor& base, const QRect& r,
   bool focus, bool hover,
   qreal opacity, AnimationMode animationMode,  TileSet::Tiles tiles, bool outline) const
      {
      if ( !r.isValid() )
            return;
      if (opacity >= 0 && (animationMode & AnimationFocus) ) {
            // calculate proper glow color based on current settings and opacity
            const QColor glow( hover ?
                               ColorUtils::mix( viewHoverBrush().brush(QPalette::Active).color(), viewFocusBrush().brush(QPalette::Active).color(), opacity ) :
                               alphaColor(  viewFocusBrush().brush(QPalette::Active).color(), opacity ) );

            holeFocused(base, glow, 0.0, 7, outline)->render(r, p, tiles);
            }
      else if (focus) {
            holeFocused(base, viewFocusBrush().brush(QPalette::Active).color(), 0.0)->render(r, p, tiles);
            }
      else if ( opacity >= 0 && ( animationMode & AnimationHover ) ) {
            // calculate proper glow color based on current settings and opacity
            const QColor glow( alphaColor(  viewHoverBrush().brush(QPalette::Active).color(), opacity ) );
            holeFocused(base, glow, 0.0, 7, outline)->render(r, p, tiles);
            }
      else if (hover) {
            holeFocused(base, viewHoverBrush().brush(QPalette::Active).color(), 0.0)->render(r, p, tiles);
            }
      else {
            hole(base, 0.0, 7, outline)->render(r, p, tiles);
            }
      }

//---------------------------------------------------------
//   holeFocused
//---------------------------------------------------------

TileSet* StyleHelper::holeFocused(const QColor& color, const QColor& glowColor, qreal shade, int size, bool outline) const {
      // FIXME must move to s/slabcache/cache/ b/c key is wrong
      Cache<TileSet>::Value* cache(m_holeFocusedCache.get(glowColor) );

      const quint64 key( (quint64(color.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size << 1 | outline );
      TileSet* tileSet = cache->object(key);

      if (!tileSet) {
            const int rsize( (int)ceil(qreal(size) * 5.0 / 7.0) );
            QImage image(rsize * 2, rsize * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);

            TileSet* holeTileSet = hole(color, shade, size, outline);
            holeTileSet->render(QRect(0, 0, 10, 10), &p);
            p.setWindow(2, 2, 10, 10);
            drawInverseGlow(p, glowColor, 3, 8, size);
            p.end();

            tileSet = new TileSet(QPixmap::fromImage(image), rsize, rsize, rsize, rsize, rsize - 1, rsize, 2, 1);

            cache->insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   scrollHole
//---------------------------------------------------------

TileSet* StyleHelper::scrollHole(const QColor& color, Qt::Orientation orientation, bool smallShadow) const {
      const quint64 key( quint64(color.rgba()) << 32 | (orientation == Qt::Horizontal ? 2 : 0) | (smallShadow ? 1 : 0) );
      TileSet* tileSet = m_scrollHoleCache.object(key);
      if (!tileSet) {
            QImage image(15, 15,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);

            const QColor dark( calcDarkColor(color) );
            const QColor light( calcLightColor(color) );
            const QColor shadow( calcShadowColor(color) );

            // use space for white border
            const QRectF r( QRect(0, 0, 15, 15) );
            const QRectF rect( r.adjusted(1, 0, -1, -1) );
            int shadowWidth(0);
            if ( smallShadow )
                  shadowWidth = (orientation == Qt::Horizontal) ? 2 : 1;
            else
                  shadowWidth = (orientation == Qt::Horizontal) ? 3 : 2;

            p.setRenderHints(QPainter::Antialiasing);
            p.setBrush(dark);
            p.setPen(Qt::NoPen);

            // base
            p.drawRoundedRect(rect, 4.5, 4.5);

            // slight shadow across the whole hole
            QLinearGradient shadowGradient(rect.topLeft(),
                                           orientation == Qt::Horizontal ? rect.bottomLeft() : rect.topRight());

            shadowGradient.setColorAt(0.0, alphaColor(shadow, 0.1));
            shadowGradient.setColorAt(0.6, Qt::transparent);
            p.setBrush(shadowGradient);
            p.drawRoundedRect(rect, 4.5, 4.5);

            // strong shadow

            // left
            QLinearGradient l1 = QLinearGradient(rect.topLeft(), rect.topLeft() + QPoint(shadowWidth, 0));
            l1.setColorAt(0.0, alphaColor(shadow, orientation == Qt::Horizontal ? 0.3 : 0.2));
            l1.setColorAt(0.5, alphaColor(shadow, orientation == Qt::Horizontal ? 0.1 : 0.1));
            l1.setColorAt(1.0, Qt::transparent);
            p.setBrush(l1);
            p.drawRoundedRect(QRectF(rect.topLeft(), rect.bottomLeft() + QPoint(shadowWidth, 0)), 4.5, 4.5);

            // right
            l1 = QLinearGradient(rect.topRight(), rect.topRight() - QPoint(shadowWidth, 0));
            l1.setColorAt(0.0, alphaColor(shadow, orientation == Qt::Horizontal ? 0.3 : 0.2));
            l1.setColorAt(0.5, alphaColor(shadow, orientation == Qt::Horizontal ? 0.1 : 0.1));
            l1.setColorAt(1.0, Qt::transparent);
            p.setBrush(l1);
            p.drawRoundedRect(QRectF(rect.topRight() - QPoint(shadowWidth, 0), rect.bottomRight()), 4.5, 4.5);

            //top
            l1 = QLinearGradient(rect.topLeft(), rect.topLeft() + QPoint(0, 3));
            l1.setColorAt(0.0, alphaColor(shadow, 0.3));
            l1.setColorAt(1.0, Qt::transparent);
            p.setBrush(l1);
            p.drawRoundedRect(QRectF(rect.topLeft(), rect.topRight() + QPoint(0, 3)), 4.5, 4.5);

            // light border
            QLinearGradient borderGradient(r.topLeft() + QPoint(0, r.height() / 2 - 1), r.bottomLeft());
            borderGradient.setColorAt(0.0, Qt::transparent);
            borderGradient.setColorAt(1.0, alphaColor(light, 0.8));
            p.setPen( QPen(borderGradient, 1.0) );
            p.setBrush(Qt::NoBrush);
            p.drawRoundedRect(r.adjusted(0.5, 0, -0.5, 0), 5.0, 5.0);
            p.end();

            tileSet = new TileSet(QPixmap::fromImage(image), 7, 7, 1, 1);
            m_scrollHoleCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   calcLightColor
//---------------------------------------------------------

const QColor& StyleHelper::calcLightColor(const QColor& color) const {
      const quint64 key( color.rgba() );
      QColor* out( m_lightColorCache.object( key ) );
      if (!out) {
            out = new QColor( highThreshold(color) ? color : ColorScheme::shade(color, ColorScheme::LightShade, _contrast) );
            m_lightColorCache.insert(key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   calcDarkColor
//---------------------------------------------------------

const QColor& StyleHelper::calcDarkColor(const QColor& color) const {
      const quint64 key( color.rgba() );
      QColor* out( m_darkColorCache.object( key ) );
      if ( !out ) {
            out = new QColor( (lowThreshold(color)) ?
                              ColorUtils::mix(calcLightColor(color), color, 0.3 + 0.7 * _contrast) :
                              ColorScheme::shade(color, ColorScheme::MidShade, _contrast) );
            m_darkColorCache.insert(key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   alphaColor
//---------------------------------------------------------

QColor StyleHelper::alphaColor(QColor color, qreal alpha) {
      if (alpha >= 0 && alpha < 1.0) {
            color.setAlphaF(alpha * color.alphaF());
            }
      return color;
      }

//---------------------------------------------------------
//   drawInverseShadow
//---------------------------------------------------------

void StyleHelper::drawInverseShadow(
      QPainter& p, const QColor& color, int pad, int size, qreal fuzz ) const {
      const qreal m( qreal(size) * 0.5 );
      const QColor shadow( calcShadowColor( color ) );
      const qreal offset( 0.8 );
      const qreal k0( (m - 2) / qreal(m + 2.0) );
      QRadialGradient shadowGradient(pad + m, pad + m + offset, m + 2);
      for (int i = 0; i < 8; i++) {
            // sinusoidal gradient
            qreal k1 = (qreal(8 - i) + k0 * qreal(i)) * 0.125;
            qreal a = (cos(3.14159 * i * 0.125) + 1.0) * 0.25;
            shadowGradient.setColorAt(k1, alphaColor(shadow, a * _shadowGain));
            }
      shadowGradient.setColorAt(k0, alphaColor(color, 0.0));
      p.setBrush(shadowGradient);
      p.drawEllipse(QRectF(pad - fuzz, pad - fuzz, size + fuzz * 2.0, size + fuzz * 2.0));
      }

//---------------------------------------------------------
//   drawInverseGlow
//---------------------------------------------------------

void StyleHelper::drawInverseGlow(
      QPainter& p, const QColor& color,
      int pad, int size, int rsize) const {
      const QRectF r(pad, pad, size, size);
      const qreal m( qreal(size) * 0.5 );

      const qreal width( 3.5 );
      const qreal bias( _glowBias * 7.0 / rsize );
      const qreal k0( (m - width) / (m - bias) );
      QRadialGradient glowGradient(pad + m, pad + m, m - bias);
      for (int i = 0; i < 8; i++) {
            // inverse parabolic gradient
            qreal k1 = (k0 * qreal(i) + qreal(8 - i)) * 0.125;
            qreal a = 1.0 - sqrt(i * 0.125);
            glowGradient.setColorAt(k1, alphaColor(color, a));

            }

      glowGradient.setColorAt(k0, alphaColor(color, 0.0));
      p.setBrush(glowGradient);
      p.drawEllipse(r);
      }

//---------------------------------------------------------
//   calcShadowColor
//---------------------------------------------------------

const QColor& StyleHelper::calcShadowColor(const QColor& color) const {
      const quint64 key( color.rgba() );
      QColor* out( m_shadowColorCache.object( key ) );
      if ( !out ) {
            out = new QColor( (lowThreshold(color)) ?
                              ColorUtils::mix( Qt::black, color, color.alphaF() ) :
                              ColorScheme::shade(ColorUtils::mix( Qt::black, color, color.alphaF() ),
                                                 ColorScheme::ShadowShade, _contrast) );
            m_shadowColorCache.insert(key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   renderMenuBackground
//---------------------------------------------------------

void StyleHelper::renderMenuBackground(QPainter* p, const QRect& clipRect, const QWidget* widget, const QColor& color) const {
      // get coordinates relative to the client area
      // this is stupid. One could use mapTo if this was taking const QWidget* and not
      // QWidget* as argument.
      const QWidget* w( widget );
      int x(0);
      int y(0);

      while ( !w->isWindow() && w != w->parentWidget() ) {
            x += w->geometry().x();
            y += w->geometry().y();
            w = w->parentWidget();
            }

      if (clipRect.isValid()) {
            p->save();
            p->setClipRegion(clipRect, Qt::IntersectClip);
            }

      // calculate upper part height
      // special tricks are needed
      // to handle both window contents and window decoration
      QRect r = w->rect();
      const int height( w->frameGeometry().height() );
      const int splitY( qMin(200, (3 * height) / 4) );

      const QRect upperRect( QRect(0, 0, r.width(), splitY) );
      const QPixmap tile(verticalGradient(color, splitY));
      p->drawTiledPixmap(upperRect, tile);

      const QRect lowerRect( 0, splitY, r.width(), r.height() - splitY );
      p->fillRect(lowerRect, backgroundBottomColor(color));

      if (clipRect.isValid())
            p->restore();
      }

//---------------------------------------------------------
//   verticalGradient
//---------------------------------------------------------

QPixmap StyleHelper::verticalGradient(const QColor& color, int height, int offset) const {
      const quint64 key((quint64(color.rgba()) << 32) | height | 0x8000);
      QPixmap* pixmap(m_backgroundCache.object(key));

      if (!pixmap) {
            QImage image(1, height,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QLinearGradient gradient(0, offset, 0, height + offset);
            gradient.setColorAt(0.0, backgroundTopColor(color));
            gradient.setColorAt(0.5, color);
            gradient.setColorAt(1.0, backgroundBottomColor(color));

            QPainter p(&image);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.fillRect(image.rect(), gradient);
            p.end();
            pixmap = new QPixmap(QPixmap::fromImage(image));
            m_backgroundCache.insert(key, pixmap);
            }
      return *pixmap;
      }

//---------------------------------------------------------
//   radialGradient
//---------------------------------------------------------

QPixmap StyleHelper::radialGradient(const QColor& color, int width, int height) const {
      const quint64 key((quint64(color.rgba()) << 32) | width | 0xb000);
      QPixmap* pixmap(m_backgroundCache.object(key));

      if (!pixmap) {
            QImage image(width, height,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QColor radialColor = backgroundRadialColor(color);
            printf("radialGradient %d %d  %d %d %d\n", width, height,
                   radialColor.red(), radialColor.green(), radialColor.blue());
            radialColor.setAlpha(255);
            QRadialGradient gradient(64, height - 64, 64);
            gradient.setColorAt(0, radialColor);
            radialColor.setAlpha(101);
            gradient.setColorAt(0.5, radialColor);
            radialColor.setAlpha(37);
            gradient.setColorAt(0.75, radialColor);
            radialColor.setAlpha(0);
            gradient.setColorAt(1, radialColor);
            QPainter p(&image);
            p.scale(width / 128.0, 1);
            p.fillRect(QRect(0, 0, 128, height), gradient);
            p.end();
            pixmap = new QPixmap(QPixmap::fromImage(image));
            m_backgroundCache.insert(key, pixmap);
            }
      return *pixmap;
      }

//---------------------------------------------------------
//   backgroundRadialColor
//---------------------------------------------------------

const QColor& StyleHelper::backgroundRadialColor(const QColor& color) const {
      const quint64 key( color.rgba() );
      QColor* out( m_backgroundRadialColorCache.object( key ) );
      if (!out) {
            if ( lowThreshold(color) )
                  out = new QColor(ColorScheme::shade(color, ColorScheme::LightShade, 0.0) );
            else if ( highThreshold(color))
                  out = new QColor(color);
            else out = new QColor(ColorScheme::shade(color, ColorScheme::LightShade, _bgcontrast) );
            m_backgroundRadialColorCache.insert( key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   renderWindowBackground
//---------------------------------------------------------

void StyleHelper::renderWindowBackground(QPainter* p, const QRect& clipRect, const QWidget* widget,
            const QWidget* window, const QColor& color, int y_shift, int gradientHeight) const {
      // get coordinates relative to the client area
      // this is stupid. One could use mapTo if this was taking const QWidget* and not
      // QWidget* as argument.

      const QWidget* w( widget );
      int x(0);
      int y(-y_shift);

      while (w != window && !w->isWindow() && w != w->parentWidget()) {
            x += w->geometry().x();
            y += w->geometry().y();
            w = w->parentWidget();
            }

      if (clipRect.isValid()) {
            p->save();
            p->setClipRegion(clipRect, Qt::IntersectClip);
            }

      // calculate upper part height
      // special tricks are needed
      // to handle both window contents and window decoration

      const QRect r = window->rect();
      int height    = window->frameGeometry().height();
      int width     = window->frameGeometry().width();

      if (y_shift > 0) {
            height -= 2 * y_shift;
            width  -= 2 * y_shift;
            }

      const int splitY(qMin(300, (3 * height) / 4) );

      // draw upper linear gradient
      const QRect upperRect(-x, -y, r.width(), splitY);
      QPixmap tile( verticalGradient(color, splitY, gradientHeight - 64) );
      p->drawTiledPixmap(upperRect, tile);

      // draw lower flat part
      const QRect lowerRect(-x, splitY - y, r.width(), r.height() - splitY - y_shift);
      p->fillRect(lowerRect, backgroundBottomColor(color));

      // draw upper radial gradient
      // WS: dont know how this should look like, but its
      // obviously wrong on MAC:
#if 0  //DEBUG
      const int radialW (qMin(600, width));
      const QRect radialRect( (r.width() - radialW) / 2 - x, -y, radialW, gradientHeight);
      if (clipRect.intersects(radialRect)) {
            tile = radialGradient(color, radialW, gradientHeight);
            p->drawPixmap(radialRect, tile);
            }
#endif
      if (clipRect.isValid())
            p->restore();
      }

//---------------------------------------------------------
//   renderDot
//---------------------------------------------------------

void StyleHelper::renderDot(QPainter* p, const QPoint& point, const QColor& baseColor) const {
      const quint64 key(baseColor.rgba());
      QPixmap* pixmap(m_dotCache.object(key));

      if (!pixmap) {
            pixmap = new QPixmap( 4 * qApp->devicePixelRatio(), 4 * qApp->devicePixelRatio());
            pixmap->fill( Qt::transparent );
            const qreal diameter( 1.8 );

            QPainter painter( pixmap );
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::NoPen);

            const QPoint center( pixmap->rect().center() );

            // light ellipse
            painter.setBrush(calcLightColor(baseColor));
            painter.drawEllipse(QRectF(center.x() - diameter / 2 + 1.0, center.y() - diameter / 2 + 1.0, diameter, diameter));

            // dark ellipse
            painter.setBrush(calcDarkColor(baseColor).darker(130));
            painter.drawEllipse(QRectF(center.x() - diameter / 2 + 0.5, center.y() - diameter / 2 + 0.5, diameter, diameter));
            painter.end();

            // store in cache
            m_dotCache.insert( key, pixmap );
            }

      p->save();
      p->translate( point - QPoint(1, 1) );
      p->setRenderHint(QPainter::Antialiasing);
      p->drawPixmap( QRect(0, 0, 4, 4), *pixmap );
      p->restore();
      }

//---------------------------------------------------------
//   drawSeparator
//---------------------------------------------------------

void StyleHelper::drawSeparator(QPainter* p, const QRect& rect, const QColor& color, Qt::Orientation orientation) const {
      QColor light(calcLightColor(color));
      QColor dark(calcDarkColor(color));

      p->save();
      p->setRenderHint(QPainter::Antialiasing, false);

      QPoint start, end, offset;
      if (orientation == Qt::Horizontal) {
            start = QPoint(rect.x(), rect.y() + rect.height() / 2 - 1);
            end = QPoint(rect.right(), rect.y() + rect.height() / 2 - 1);
            offset = QPoint(0, 1);
            }
      else {
            start = QPoint(rect.x() + rect.width() / 2 - 1, rect.y());
            end = QPoint(rect.x() + rect.width() / 2 - 1, rect.bottom());
            offset = QPoint(1, 0);
            light.setAlpha(150);
            }

      QLinearGradient lg(start, end);
      lg.setColorAt(0.3, dark);
      lg.setColorAt(0.7, dark);
      dark.setAlpha(0);
      lg.setColorAt(0.0, dark);
      lg.setColorAt(1.0, dark);
      p->setPen(QPen(lg, 1));

      if (orientation == Qt::Horizontal)
            p->drawLine(start, end);
      else
            p->drawLine(start + offset, end + offset);

      lg = QLinearGradient(start, end);
      lg.setColorAt(0.3, light);
      lg.setColorAt(0.7, light);
      light.setAlpha(0);
      lg.setColorAt(0.0, light);
      lg.setColorAt(1.0, light);
      p->setPen(QPen(lg, 1));

      if (orientation == Qt::Horizontal)
            p->drawLine(start + offset, end + offset);
      else {
            p->drawLine(start, end);
            p->drawLine(start + offset * 2, end + offset * 2);
            }
      p->restore();
      }

//---------------------------------------------------------
//   decoColor
//---------------------------------------------------------

const QColor& StyleHelper::decoColor(const QColor& background, const QColor& color) const {
      const quint64 key( (quint64( background.rgba() ) << 32) | color.rgba() );
      QColor* out( m_decoColorCache.object( key ) );
      if (!out) {
            out = new QColor(ColorUtils::mix( background, color, 0.4 + 0.8 * _contrast ) );
            m_decoColorCache.insert( key, out );
            }
      return *out;
      }

//---------------------------------------------------------
//   roundedMask
//---------------------------------------------------------

QRegion StyleHelper::roundedMask( const QRect& r, int left, int right, int top, int bottom ) const {
      // get rect geometry
      int x, y, w, h;
      r.getRect(&x, &y, &w, &h);

      QRegion mask(x + 4 * left, y + 0 * top, w - 4 * (left + right), h - 0 * (top + bottom));
      mask += QRegion(x + 0 * left, y + 4 * top, w - 0 * (left + right), h - 4 * (top + bottom));
      mask += QRegion(x + 2 * left, y + 1 * top, w - 2 * (left + right), h - 1 * (top + bottom));
      mask += QRegion(x + 1 * left, y + 2 * top, w - 1 * (left + right), h - 2 * (top + bottom));
      return mask;
      }

//---------------------------------------------------------
//   fillSlab
//---------------------------------------------------------

void StyleHelper::fillSlab(QPainter& p, const QRect& rect, int size) const {
      const qreal s( qreal(size) * (3.6 + (0.5 * _slabThickness)) / 7.0 );
      const QRectF r( QRectF(rect).adjusted(s, s, -s, -s) );
      if ( !r.isValid() )
            return;
      p.drawRoundedRect( r, s, s );
      }

//---------------------------------------------------------
//   progressBarIndicator
//---------------------------------------------------------

QPixmap StyleHelper::progressBarIndicator(const QPalette& pal, const QRect& rect) const {
      const QColor highlight( pal.color( QPalette::Highlight ) );
      const quint64 key( (quint64(highlight.rgba()) << 32) | (rect.width() << 16 ) | (rect.height() ) );

      QPixmap* pixmap = m_progressBarCache.object(key);
      if (!pixmap) {
            QRect local( rect );
            local.adjust( -1, -2, 1, 1 );

            // set topLeft corner to 0.0
            local.translate( -local.topLeft() );

            QImage image(local.size(),  QImage::Format_ARGB32_Premultiplied);
            image.fill( Qt::transparent );

            QPainter p(&image);
            p.setRenderHints( QPainter::Antialiasing );
            p.setBrush(Qt::NoBrush);

            local.adjust( 1, 1, -1, -1 );
            const QColor lhighlight( calcLightColor(highlight) );
            const QColor color( pal.color(QPalette::Active, QPalette::Window) );
            const QColor light( calcLightColor(color) );
            const QColor dark( calcDarkColor(color) );
            const QColor shadow( calcShadowColor(color) );

            // shadow
            p.setPen(QPen(alphaColor(shadow, 0.6), 0.6));
            p.drawRoundedRect(QRectF(local).adjusted( 0.5, -0.5, 0.5, 1.5), 2, 2 );

            // fill
            p.setPen(Qt::NoPen);
            p.setBrush(ColorUtils::mix(highlight, dark, 0.2));
            p.drawRect(local.adjusted(1, 0, -1, 0 ) );

            // fake radial gradient
            local.adjust( 0, 0, -1, 0 );
            QImage pm(local.size(),  QImage::Format_ARGB32_Premultiplied);
            pm.fill(Qt::transparent);
                  {
                  QRectF pmRect = pm.rect();
                  QLinearGradient mask(pmRect.topLeft(), pmRect.topRight());
                  mask.setColorAt(0.0, Qt::transparent);
                  mask.setColorAt(0.4, Qt::black);
                  mask.setColorAt(0.6, Qt::black);
                  mask.setColorAt(1.0, Qt::transparent);

                  QLinearGradient radial(pmRect.topLeft(), pmRect.bottomLeft());
                  radial.setColorAt(0.0, ColorUtils::mix(lhighlight, light, 0.3));
                  radial.setColorAt(0.5, Qt::transparent);
                  radial.setColorAt(0.6, Qt::transparent);
                  radial.setColorAt(1.0, ColorUtils::mix(lhighlight, light, 0.3));

                  QPainter pp(&pm);
                  pp.fillRect(pm.rect(), mask);
                  pp.setCompositionMode(QPainter::CompositionMode_SourceIn);
                  pp.fillRect(pm.rect(), radial);
                  pp.end();
                  }

            p.drawPixmap( QPoint(1, 1), QPixmap::fromImage(pm));

            // bevel
            p.setRenderHint(QPainter::Antialiasing, false);
            QLinearGradient bevel(local.topLeft(), local.bottomLeft());
            bevel.setColorAt(0, lhighlight);
            bevel.setColorAt(0.5, highlight);
            bevel.setColorAt(1, calcDarkColor(highlight));
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(bevel, 1));
            p.drawRoundedRect(local, 2, 2);

            // bright top edge
            QLinearGradient lightHl(local.topLeft(), local.topRight());
            lightHl.setColorAt(0, Qt::transparent);
            lightHl.setColorAt(0.5, ColorUtils::mix(highlight, light, 0.8));
            lightHl.setColorAt(1, Qt::transparent);
            p.setPen(QPen(lightHl, 1));
            p.drawLine(local.topLeft(), local.topRight());
            p.end();
            pixmap = new QPixmap(QPixmap::fromImage(image));
            m_progressBarCache.insert(key, pixmap);
            }
      return *pixmap;
      }

//---------------------------------------------------------
//   dialSlab
//---------------------------------------------------------

QPixmap StyleHelper::dialSlab(const QColor& color, qreal shade, int size) const {
      Cache<QPixmap>::Value* cache = m_dialSlabCache.get(color);

      const quint64 key( (quint64(256.0 * shade) << 24) | size );
      QPixmap* pixmap = cache->object(key);
      if (!pixmap) {
            QImage image(size, size,  QImage::Format_ARGB32_Premultiplied);
            image.fill( Qt::transparent );

            const QRectF rect(image.rect() );

            QPainter p(&image);
            p.setPen( Qt::NoPen );
            p.setRenderHints(QPainter::Antialiasing);

            // colors
            const QColor base(ColorUtils::shade(color, shade) );
            const QColor light(ColorUtils::shade(calcLightColor(color), shade) );
#if 0 // yet (?) unused
            const QColor dark(ColorUtils::shade(calcDarkColor(color), shade) );
#endif
            const QColor mid(ColorUtils::shade(calcMidColor(color), shade) );
            const QColor shadow( calcShadowColor(color) );

            // shadow
            drawShadow( p, shadow, rect.width() );

            const qreal baseOffset = 3.5;
                  {
                  //plain background
                  QLinearGradient lg( 0, baseOffset - 0.5 * rect.height(), 0, baseOffset + rect.height() );
                  lg.setColorAt( 0, light );
                  lg.setColorAt( 0.8, base );
                  p.setBrush( lg );
                  const qreal offset( baseOffset );
                  p.drawEllipse( rect.adjusted( offset, offset, -offset, -offset ) );
                  }

                  {
                  // outline circle
                  const qreal penWidth( 0.7 );
                  QLinearGradient lg( 0, baseOffset, 0, baseOffset + 2 * rect.height() );
                  lg.setColorAt( 0, light );
                  lg.setColorAt( 1, mid );
                  p.setBrush( Qt::NoBrush );
                  p.setPen( QPen( lg, penWidth ) );
                  const qreal offset( baseOffset + 0.5 * penWidth );
                  p.drawEllipse( rect.adjusted( offset, offset, -offset, -offset ) );
                  }
            pixmap = new QPixmap(QPixmap::fromImage(image));
            cache->insert(key, pixmap);
            }

      return *pixmap;
      }

//---------------------------------------------------------
//   dialSlabFocused
//---------------------------------------------------------

QPixmap StyleHelper::dialSlabFocused(const QColor& color, const QColor& glowColor, qreal shade, int size) const {
      return QPixmap();

      Cache<QPixmap>::Value* cache =  m_dialSlabCache.get(color);

      const quint64 key((quint64(glowColor.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size);
      QPixmap* pixmap = cache->object(key);
      if (!pixmap) {
            QImage image(size, size,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QRectF rect(image.rect());

            QPainter p(&image);
            p.setPen( Qt::NoPen );
            p.setRenderHints(QPainter::Antialiasing);

            // colors
            const QColor base(ColorUtils::shade(color, shade));
            const QColor light(ColorUtils::shade(calcLightColor(color), shade));
#if 0 // yet (?) unused
            const QColor dark(ColorUtils::shade(calcDarkColor(color), shade));
#endif
            const QColor mid(ColorUtils::shade(calcMidColor(color), shade));
            const QColor shadow(calcShadowColor(color));

            // shadow
            drawShadow(p, shadow, rect.width());
            drawOuterGlow(p, glowColor, rect.width());

            const qreal baseOffset(3.5);
                  {
                  //plain background
                  QLinearGradient lg(0, baseOffset - 0.5 * rect.height(), 0, baseOffset + rect.height());
                  lg.setColorAt(0, light);
                  lg.setColorAt(0.8, base);
                  p.setBrush(lg);
                  const qreal offset(baseOffset);
                  p.drawEllipse(rect.adjusted( offset, offset, -offset, -offset));
                  }

                  {
                  // outline circle
                  const qreal penWidth(0.7);
                  QLinearGradient lg(0, baseOffset, 0, baseOffset + 2 * rect.height());
                  lg.setColorAt(0, light);
                  lg.setColorAt(1, mid);
                  p.setBrush(Qt::NoBrush);
                  p.setPen(QPen(lg, penWidth));
                  const qreal offset(baseOffset + 0.5 * penWidth );
                  p.drawEllipse(rect.adjusted( offset, offset, -offset, -offset));
                  }

            pixmap = new QPixmap(QPixmap::fromImage(image));
            cache->insert(key, pixmap);
            }
      return *pixmap;
      }

//---------------------------------------------------------
//   roundSlab
//---------------------------------------------------------

QPixmap StyleHelper::roundSlab(const QColor& color, qreal shade, int size) const {
      Cache<QPixmap>::Value* cache(m_roundSlabCache.get(color));

      const quint64 key( (quint64(256.0 * shade) << 24) | size);
      QPixmap* pixmap = cache->object(key);

      if (!pixmap) {
            QImage image(size * 3 * qApp->devicePixelRatio(), size * 3 * qApp->devicePixelRatio(),  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(0, 0, 21, 21);
            // shadow
            drawShadow(p, calcShadowColor(color), 21);
            drawRoundSlab(p, color, shade);
            p.end();
            pixmap = new QPixmap(QPixmap::fromImage(image));
            cache->insert(key, pixmap);
            }
      return *pixmap;
      }

//---------------------------------------------------------
//   roundSlabFocused
//---------------------------------------------------------

QPixmap StyleHelper::roundSlabFocused(const QColor& color, const QColor& glowColor, qreal shade, int size) const {
      Cache<QPixmap>::Value* cache(m_roundSlabCache.get(color));

      const quint64 key((quint64(glowColor.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size);
      QPixmap* pixmap = cache->object(key);

      if (!pixmap) {
            QImage image(size * 3 * qApp->devicePixelRatio(), size * 3 * qApp->devicePixelRatio(),  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);
            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(0, 0, 21, 21);
            drawShadow(p, calcShadowColor(color), 21);
            drawOuterGlow(p, glowColor, 21);
            drawRoundSlab(p, color, shade);
            p.end();
            pixmap = new QPixmap(QPixmap::fromImage(image));
            cache->insert(key, pixmap);
            }
      return *pixmap;
      }

//---------------------------------------------------------
//   slabFocused
//---------------------------------------------------------

TileSet* StyleHelper::slabFocused(const QColor& color, const QColor& glowColor, qreal shade, int size) const {
      Cache<TileSet>::Value* cache( m_slabCache.get(color) );

      const quint64 key( (quint64(glowColor.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size );
      TileSet* tileSet = cache->object(key);

      const qreal hScale( 1 );
      const int hSize( size * hScale );
      const int vSize( size );

      if (!tileSet) {
            QImage image(hSize * 2, vSize * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);

            const int fixedSize = 14;
            p.setWindow(0, 0, fixedSize * hScale, fixedSize);

            // draw all components
            if (color.isValid())
                  drawShadow(p, calcShadowColor(color), 14);
            if (glowColor.isValid())
                  drawOuterGlow(p, glowColor, 14);
            if (color.isValid())
                  drawSlab(p, color, shade);
            p.end();
            tileSet = new TileSet(QPixmap::fromImage(image), hSize, vSize, hSize, vSize, hSize - 1, vSize, 2, 1);
            cache->insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   slabSunken
//---------------------------------------------------------

TileSet* StyleHelper::slabSunken(const QColor& color, qreal shade, int size) const {
      const quint64 key( (quint64(color.rgba()) << 32) );
      TileSet* tileSet = m_slabSunkenCache.object(key);

      if (!tileSet) {
            QImage image(size * 2, size * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(0, 0, 14, 14);

            // slab
            drawSlab(p, color, shade);

            // shadow
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
            drawInverseShadow(p, calcShadowColor(color), 3, 8, 0.0);

            p.end();
            tileSet = new TileSet(QPixmap::fromImage(image), size, size, size, size, size - 1, size, 2, 1);
            m_slabSunkenCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   slabInverted
//---------------------------------------------------------

TileSet* StyleHelper::slabInverted(const QColor& color, qreal shade, int size) const {
      const quint64 key( (quint64(color.rgba()) << 32) );
      TileSet* tileSet = m_slabInvertedCache.object(key);

      if (!tileSet) {
            QImage image(size * 2, size * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(0, 0, 14, 14);

            const QColor base(ColorUtils::shade(color, shade) );
            const QColor light(ColorUtils::shade(calcLightColor(color), shade) );
            const QColor dark(ColorUtils::shade(calcDarkColor(color), shade) );

            // bevel, part 2
            QLinearGradient bevelGradient2(0, 8, 0, -8);
            bevelGradient2.setColorAt(0.0, light);
            bevelGradient2.setColorAt(0.9, base);
            p.setBrush(bevelGradient2);
            p.drawEllipse(QRectF(2.6, 2.6, 8.8, 8.8));

            // bevel, part 1
            qreal y = ColorUtils::luma(base);
            qreal yl = ColorUtils::luma(light);
            qreal yd = ColorUtils::luma(dark);
            QLinearGradient bevelGradient1(0, 7, 0, 4);
            bevelGradient1.setColorAt(0.0, light);
            bevelGradient1.setColorAt(0.9, dark);

            if (y < yl && y > yd) {
                  // no middle when color is very light/dark
                  bevelGradient1.setColorAt(0.5, base);
                  }

            p.setBrush(bevelGradient1);
            p.drawEllipse(QRectF(3.4, 3.4, 7.2, 7.2));

            // inside mask
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setBrush(QBrush(Qt::black));
            p.drawEllipse(QRectF(4.0, 4.0, 6.0, 6.0));

            // shadow
            p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
            drawInverseShadow(p, calcShadowColor(color), 4, 6, 0.5);
            p.end();
            tileSet = new TileSet(QPixmap::fromImage(image), size, size, size, size, size - 1, size, 2, 1);
            m_slabInvertedCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   slab
//---------------------------------------------------------

TileSet* StyleHelper::slab(const QColor& color, qreal shade, int size) const {
      Cache<TileSet>::Value* cache( m_slabCache.get(color) );
      const quint64 key( ((int)(256.0 * shade)) << 24 | size );
      TileSet* tileSet( cache->object( key ) );

      if (!tileSet) {
            QImage image(size * 2, size * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(0, 0, 14, 14);

            // shadow
            drawShadow(p, calcShadowColor(color), 14);
            drawSlab(p, color, shade);

            p.end();

            tileSet = new TileSet(QPixmap::fromImage(image), size, size, size, size, size - 1, size, 2, 1);

            cache->insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   drawFloatFrame
//---------------------------------------------------------

void StyleHelper::drawFloatFrame(QPainter* p, const QRect r,
                                 const QColor& color, bool drawUglyShadow, bool isActive,
                                 const QColor& frameColor, TileSet::Tiles tiles) const {
      p->save();
      p->setRenderHint(QPainter::Antialiasing);
      const QRect frame( r.adjusted(1, 1, -1, -1) );
      int x, y, w, h;
      frame.getRect(&x, &y, &w, &h);

      QColor light( calcLightColor(backgroundTopColor(color)) );
      QColor dark( calcLightColor(backgroundBottomColor(color)) );
      p->setBrush(Qt::NoBrush);

      if (drawUglyShadow) {
            if (isActive) {
                  //window active - it's a glow - not a shadow
                  const QColor glow(ColorUtils::mix(QColor(128, 128, 128), frameColor, 0.7) );
                  p->setPen(glow);

                  if (tiles & TileSet::Top) {
                        p->drawLine(QPointF(x + 4, y - 0.5), QPointF(x + w - 4, y - 0.5));
                        p->drawArc(QRectF(x - 0.5, y - 0.5, 11, 11), 90 * 16, 90 * 16);
                        p->drawArc(QRectF(x + w - 11 + 0.5, y - 0.5, 11, 11), 0, 90 * 16);
                        }

                  if ( tiles & TileSet::Left )
                        p->drawLine(QPointF(x - 0.5, y + 4), QPointF(x - 0.5, y + h - 4));
                  if ( tiles & TileSet::Right )
                        p->drawLine(QPointF(x + w + 0.5, y + 4), QPointF(x + w + 0.5, y + h - 4));

                  if ( tiles & TileSet::Bottom ) {
                        if ( tiles & TileSet::Left )
                              p->drawArc(QRectF(x - 0.5, y + h - 11 + 0.5, 11, 11), 180 * 16, 90 * 16);
                        if ( tiles & TileSet::Right )
                              p->drawArc(QRectF(x + w - 11 + 0.5, y + h - 11 + 0.5, 11, 11), 270 * 16, 90 * 16);
                        p->drawLine(QPointF(x + 4, y + h + 0.5), QPointF(x + w - 4, y + h + 0.5));
                        }

                  light = ColorUtils::mix(light, frameColor);
                  dark = ColorUtils::mix(dark, frameColor);
                  }
            else {
                  // window inactive - draw something resembling shadow
                  // fully desaturate
                  const QColor shadow(ColorUtils::darken(color, 0.0, 0.0));

                  if (tiles & TileSet::Top) {
                        p->setPen(ColorUtils::darken(shadow, 0.2));
                        p->drawLine(QPointF(x + 4, y - 0.5), QPointF(x + w - 4, y - 0.5));
                        if (tiles & TileSet::Left)
                              p->drawArc(QRectF(x - 0.5, y - 0.5, 11, 11), 90 * 16, 90 * 16);
                        if (tiles & TileSet::Right)
                              p->drawArc(QRectF(x + w - 11 + 0.5, y - 0.5, 11, 11), 0, 90 * 16);
                        }

                  p->setPen(ColorUtils::darken(shadow, 0.35));
                  if (tiles & TileSet::Left)
                        p->drawLine(QPointF(x - 0.5, y + 4), QPointF(x - 0.5, y + h - 4));
                  if (tiles & TileSet::Right)
                        p->drawLine(QPointF(x + w + 0.5, y + 4), QPointF(x + w + 0.5, y + h - 4));

                  if (tiles & TileSet::Bottom) {
                        p->setPen(ColorUtils::darken(shadow, 0.45));
                        if (tiles & TileSet::Left)
                              p->drawArc(QRectF(x - 0.5, y + h - 11 + 0.5, 11, 11), 180 * 16, 90 * 16);

                        if (tiles & TileSet::Right)
                              p->drawArc(QRectF(x + w - 11 + 0.5, y + h - 11 + 0.5, 11, 11), 270 * 16, 90 * 16);
                        p->setPen(ColorUtils::darken(shadow, 0.6));
                        p->drawLine(QPointF(x + 4, y + h + 0.5), QPointF(x + w - 4, y + h + 0.5));
                        }
                  }
            }

      // top frame
      if (tiles & TileSet::Top) {
            p->setPen(QPen(light, 0.8));
            p->drawLine(QPointF(x + 4, y + 0.6), QPointF(x + w - 4, y + 0.6));
            }

      // corner and side frames
      // sides are drawn even if Top only is selected, but with a different gradient
      if (h >= 4 + 1.5) {
            QLinearGradient lg(0.0, y + 1.5, 0.0, y + h - 4);
            lg.setColorAt(0, light);
            lg.setColorAt(1, alphaColor(light, 0) );

            if (h > 20.5)
                  lg.setColorAt(qMax(0.0, 1.0 - 12.0 / (h - 5.5)), alphaColor(light, 0.5));
            else if (h > 8.5)
                  lg.setColorAt(qMax(0.0, 3.0 / (h - 5.5)), alphaColor(light, 0.5));

            p->setPen(QPen(lg, 0.8));
            if (tiles & TileSet::Left )
                  p->drawLine(QPointF(x + 0.6, y + 4), QPointF(x + 0.6, y + h - 4));
            if (tiles & TileSet::Right )
                  p->drawLine(QPointF(x + w - 0.6, y + 4), QPointF(x + w - 0.6, y + h - 4));
            }
      if (tiles & TileSet::Top ) {
            const qreal offset = 0.5;
            const qreal arc(7.0);
            p->drawArc(QRectF(x + offset, y + offset, arc, arc), 90 * 16, 90 * 16);
            p->drawArc(QRectF(x + w - arc - offset, y + offset, arc, arc), 0, 90 * 16);
            }
      p->restore();
      }

//---------------------------------------------------------
//   roundCorner
//---------------------------------------------------------

TileSet* StyleHelper::roundCorner(const QColor& color, int size) const {
      const quint64 key( (quint64(color.rgba()) << 32) | size );
      TileSet* tileSet = m_cornerCache.object(key);

      if (!tileSet) {
            QImage image(size * 2, size * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHint(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);

            QLinearGradient lg = QLinearGradient(0.0, size - 4.5, 0.0, size + 4.5);
            lg.setColorAt(0.0, calcLightColor( backgroundTopColor(color) ));
            lg.setColorAt(0.51, backgroundBottomColor(color) );
            lg.setColorAt(1.0, backgroundBottomColor(color) );

            // draw ellipse.
            p.setBrush( lg );
            p.drawEllipse( QRectF( size - 4, size - 4, 8, 8 ) );

            // mask
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setBrush( Qt::black );
            p.drawEllipse( QRectF( size - 3, size - 3, 6, 6 ) );

            tileSet = new TileSet(QPixmap::fromImage(image), size, size, 1, 1);
            m_cornerCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   slope
//---------------------------------------------------------

TileSet* StyleHelper::slope(const QColor& color, qreal shade, int size) const {
      const quint64 key((quint64(color.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size);
      TileSet* tileSet = m_slopeCache.object(key);

      if (!tileSet) {
            QImage image(size * 4, size * 4,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setPen(Qt::NoPen);

            //lasconic: add bottom, remove gradient
            // edges
            TileSet* slabTileSet = slab(color, shade, size);
            slabTileSet->render(QRect(0, 0, size * 4, size * 4), &p,
                                TileSet::Left | TileSet::Right | TileSet::Top | TileSet::Bottom);
            
            //slabTileSet->render(QRect(0, 0, size * 4, size * 5), &p,
            //                    TileSet::Left | TileSet::Right | TileSet::Top);

            //p.setWindow(0, 0, 28, 28);

            // bottom
            /*QColor light = ColorUtils::shade(calcLightColor(color), shade);
            QLinearGradient fillGradient(0, -28, 0, 28);
            light.setAlphaF(0.4);
            fillGradient.setColorAt(0.0, light);
            light.setAlphaF(0.0);
            fillGradient.setColorAt(1.0, light);
            p.setBrush(fillGradient);
            p.setCompositionMode(QPainter::CompositionMode_DestinationOver);
            p.drawRect(3, 9, 22, 17);*/

            // fade bottom
            /*QLinearGradient maskGradient(0, 7, 0, 28);
            maskGradient.setColorAt(0.0, Qt::black);
            maskGradient.setColorAt(1.0, Qt::transparent);

            p.setBrush(maskGradient);
            p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            p.drawRect(0, 9, 28, 19);
            p.end();*/

            tileSet = new TileSet(QPixmap::fromImage(image), size, size, size * 2, 2);
            m_slopeCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   selection
//---------------------------------------------------------

TileSet* StyleHelper::selection(const QColor& color, int height, bool custom) const {
      const quint64 key( (quint64(color.rgba()) << 32) | (height << 1) | custom );
      TileSet* tileSet = m_selectionCache.object(key);
      if (!tileSet) {
            const qreal rounding( 2.5 );

            QImage image(32 + 16, height,  QImage::Format_ARGB32_Premultiplied);
            image.fill( Qt::transparent );

            QRect r(image.rect().adjusted(0, 0, -1, -1));

            QPainter p(&image);
            p.setRenderHint(QPainter::Antialiasing);
            p.translate(.5, .5);

                  {
                  // background
                  QPainterPath path;
                  path.addRoundedRect(r, rounding, rounding);

                  // items with custom background brushes always have their background drawn
                  // regardless of whether they are hovered or selected or neither so
                  // the gradient effect needs to be more subtle
                  const int lightenAmount( custom ? 110 : 130 );
                  QLinearGradient gradient(0, 0, 0, r.bottom());
                  gradient.setColorAt(0, color.lighter(lightenAmount));
                  gradient.setColorAt(1, color);
                  p.setPen(QPen(color, 1));
                  p.setBrush(gradient);
                  p.drawPath(path);
                  }

                  {
                  // contrast pixel
                  QPainterPath path;
                  path.addRoundedRect(r.adjusted(1, 1, -1, -1), rounding - 1, rounding - 1);
                  p.strokePath(path, QPen(QColor(255, 255, 255, 64), 1));
                  }

            p.end();
            tileSet = new TileSet(QPixmap::fromImage(image), 8, 0, 32, height );
            m_selectionCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   slitFocused
//---------------------------------------------------------

TileSet* StyleHelper::slitFocused(const QColor& glowColor) const {
      const quint64 key( (quint64(glowColor.rgba()) << 32) );
      TileSet* tileSet = m_slitCache.object(key);

      if (!tileSet) {
            QImage image(9, 9,  QImage::Format_ARGB32_Premultiplied);
            QPainter p;

            image.fill(Qt::transparent);

            p.begin(&image);
            p.setPen(Qt::NoPen);
            p.setRenderHint(QPainter::Antialiasing);
            QRadialGradient rg = QRadialGradient(4.5, 4.5, 4.5, 4.5, 4.5);

            QColor tmpColor = glowColor;
            tmpColor.setAlpha(180 * glowColor.alphaF());
            rg.setColorAt(0.75, tmpColor);
            tmpColor.setAlpha(0);
            rg.setColorAt(0.90, tmpColor);
            rg.setColorAt(0.4, tmpColor);
            p.setBrush(rg);
            p.drawEllipse(QRectF(0, 0, 9, 9));

            p.end();
            tileSet = new TileSet(QPixmap::fromImage(image), 4, 4, 1, 1);
            m_slitCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   drawSlab
//---------------------------------------------------------

void StyleHelper::drawSlab(QPainter& p, const QColor& color, qreal shade) const {
      const QColor light(ColorUtils::shade(calcLightColor(color), shade) );
      const QColor base( alphaColor( light, 0.85 ) );
      const QColor dark(ColorUtils::shade(calcDarkColor(color), shade) );

      // bevel, part 1
      p.save();
      const qreal y(ColorUtils::luma(base) );
      const qreal yl(ColorUtils::luma(light) );
      const qreal yd(ColorUtils::luma(dark) );
      QLinearGradient bevelGradient1(0, 7, 0, 11);
      bevelGradient1.setColorAt(0.0, light);
      if (y < yl && y > yd) {
            // no middle when color is very light/dark
            bevelGradient1.setColorAt(0.5, base);
            }

      bevelGradient1.setColorAt(0.9, base);
      p.setBrush(bevelGradient1);
      p.drawRoundedRect(QRectF(3.0, 3.0, 8.0, 8.0), 3.5, 3.5 );

      // bevel, part 2
      if (_slabThickness > 0.0) {
            QLinearGradient bevelGradient2(0, 6, 0, 19);
            bevelGradient2.setColorAt(0.0, light);
            bevelGradient2.setColorAt(0.9, base);
            p.setBrush(bevelGradient2);
            p.drawEllipse(QRectF(3.6, 3.6, 6.8, 6.8));
            }

      // inside mask
      p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
      p.setBrush(Qt::black);

      const qreal ic = 3.6 + 0.5 * _slabThickness;
      const qreal is = 6.8 - (2.0 * 0.5 * _slabThickness);
      p.drawEllipse(QRectF(ic, ic, is, is));
      p.restore();
      }

//---------------------------------------------------------
//   drawShadow
//---------------------------------------------------------

void StyleHelper::drawShadow(QPainter& p, const QColor& color, int size) const {
      const qreal m( qreal(size - 2) * 0.5 );
      const qreal offset( 0.8 );
      const qreal k0( (m - 4.0) / m );

      QRadialGradient shadowGradient(m + 1.0, m + offset + 1.0, m);
      for (int i = 0; i < 8; i++) {
            // sinusoidal gradient
            const qreal k1( (k0 * qreal(8 - i) + qreal(i)) * 0.125 );
            const qreal a((cos(3.14159 * i * 0.125) + 1.0) * 0.30 );
            shadowGradient.setColorAt(k1, alphaColor(color, a * _shadowGain));
            }

      shadowGradient.setColorAt(1.0, alphaColor(color, 0.0));
      p.save();
      p.setBrush(shadowGradient);
      p.drawEllipse(QRectF(0, 0, size, size));
      p.restore();
      }

//---------------------------------------------------------
//   drawOuterGlow
//---------------------------------------------------------

void StyleHelper::drawOuterGlow(QPainter& p, const QColor& color, int size) const {
      const QRectF r(.0, .0, size, size);
      const qreal m = qreal(size) * 0.5;
      const qreal width = 3.0;

      const qreal bias(_glowBias * qreal(14) / size);

      // k0 is located at width - bias from the outer edge
      const qreal gm(m + bias - 0.9);
      const qreal k0((m - width + bias) / gm);
      QRadialGradient glowGradient(m, m, gm);
      for (int i = 0; i < 8; i++) {
            // k1 grows linearly from k0 to 1.0
            const qreal k1( k0 + qreal(i) * (1.0 - k0) / 8.0 );

            // a folows sqrt curve
            const qreal a( 1.0 - sqrt(qreal(i) / 8) );
            glowGradient.setColorAt(k1, alphaColor(color, a));
            }
      // glow
      p.save();
      p.setBrush(glowGradient);
      p.drawEllipse(r);

      // inside mask
      p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
      p.setBrush(Qt::black);
      p.drawEllipse(r.adjusted(width, width, -width, -width));
      p.restore();
      }

//---------------------------------------------------------
//   drawRoundSlab
//---------------------------------------------------------

void StyleHelper::drawRoundSlab(QPainter& p, const QColor& color, qreal shade) const {
      p.save();

      // colors
      const QColor base(ColorUtils::shade(color, shade) );
      const QColor light(ColorUtils::shade(calcLightColor(color), shade) );
#if 0 // yet (?) unused
      const QColor dark(ColorUtils::shade(calcDarkColor(color), shade) );
#endif

      // bevel, part 1
      QLinearGradient bevelGradient1(0, 10, 0, 18);
      bevelGradient1.setColorAt(0.0, light);
      bevelGradient1.setColorAt(0.9, alphaColor( light, 0.85 ) );
      p.setBrush(bevelGradient1);
      p.drawEllipse(QRectF(3.0, 3.0, 15.0, 15.0));

      // bevel, part 2
      if (_slabThickness > 0.0) {
            QLinearGradient bevelGradient2(0, 7, 0, 28);
            bevelGradient2.setColorAt(0.0, light);
            bevelGradient2.setColorAt(0.9, base);
            p.setBrush(bevelGradient2);
            p.drawEllipse(QRectF(3.6, 3.6, 13.8, 13.8));
            }

      // inside
      QLinearGradient innerGradient(0, -17, 0, 20);
      innerGradient.setColorAt(0.0, light);
      innerGradient.setColorAt(1.0, base);
      p.setBrush(innerGradient);
      const qreal ic( 3.6 + _slabThickness );
      const qreal is( 13.8 - 2.0 * _slabThickness );
      p.drawEllipse(QRectF(ic, ic, is, is));
      p.restore();
      }

//---------------------------------------------------------
//   groove
//---------------------------------------------------------

TileSet* StyleHelper::groove(const QColor& color, qreal shade, int size) const {
      const quint64 key((quint64(color.rgba()) << 32) | (quint64(256.0 * shade) << 24) | size);
      TileSet* tileSet = m_grooveCache.object(key);

      if (!tileSet) {
            const int rsize((int)ceil(qreal(size) * 3.0 / 7.0));
            QImage image(rsize * 2, rsize * 2,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            p.setWindow(2, 2, 6, 6);

            // hole mask
            p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
            p.setBrush(Qt::black);
            p.drawEllipse(4, 4, 2, 2);

            // shadow
            p.setCompositionMode(QPainter::CompositionMode_SourceOver);
            drawInverseShadow(p, calcShadowColor(color), 3, 4, 0.0);

            p.end();

            tileSet = new TileSet(QPixmap::fromImage(image), rsize, rsize, rsize, rsize, rsize - 1, rsize, 2, 1);

            m_grooveCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   mergePalettes
//---------------------------------------------------------

QPalette StyleHelper::mergePalettes( const QPalette& source, qreal ratio ) const
      {
      QPalette out(source );
      out.setColor(QPalette::Background, ColorUtils::mix( source.color( QPalette::Active, QPalette::Background ), source.color( QPalette::Disabled, QPalette::Background ), 1.0 - ratio ) );
      out.setColor(QPalette::Highlight, ColorUtils::mix( source.color( QPalette::Active, QPalette::Highlight ), source.color( QPalette::Disabled, QPalette::Highlight ), 1.0 - ratio ) );
      out.setColor(QPalette::WindowText, ColorUtils::mix( source.color( QPalette::Active, QPalette::WindowText ), source.color( QPalette::Disabled, QPalette::WindowText ), 1.0 - ratio ) );
      out.setColor(QPalette::ButtonText, ColorUtils::mix( source.color( QPalette::Active, QPalette::ButtonText ), source.color( QPalette::Disabled, QPalette::ButtonText ), 1.0 - ratio ) );
      out.setColor(QPalette::Text, ColorUtils::mix( source.color( QPalette::Active, QPalette::Text ), source.color( QPalette::Disabled, QPalette::Text ), 1.0 - ratio ) );
      out.setColor(QPalette::Button, ColorUtils::mix( source.color( QPalette::Active, QPalette::Button ), source.color( QPalette::Disabled, QPalette::Button ), 1.0 - ratio ) );
      return out;
      }

//---------------------------------------------------------
//   dockFrame
//---------------------------------------------------------

TileSet* StyleHelper::dockFrame(const QColor& color, int w) {
      const quint64 key( quint64(color.rgba()) << 32 | w );
      TileSet* tileSet = m_dockFrameCache.object(key);
      if (!tileSet) {
            // width should be odd
            if (!w & 1)
                  --w;

            // fixed height
            const int h = 9;

            QImage pm(w, h,  QImage::Format_ARGB32_Premultiplied);
            pm.fill(Qt::transparent);

            QPainter p(&pm);
            p.save();
            p.setRenderHints(QPainter::Antialiasing);
            p.setBrush(Qt::NoBrush);
            p.translate(0.5, 0.5);
            QRectF rect(0.5, 0.5, w - 0.5, h - 0.);

            QColor light = calcLightColor(color);
            QColor dark = calcDarkColor(color);

                  {
                  // left and right border
                  QLinearGradient lg(QPoint(0, 0), QPoint(w, 0));
                  lg.setColorAt(0.0, alphaColor(light, 0.6));
                  lg.setColorAt(0.1, Qt::transparent);
                  lg.setColorAt(0.9, Qt::transparent);
                  lg.setColorAt(1.0, alphaColor(light, 0.6));
                  p.setPen(QPen(lg, 1));
                  p.drawRoundedRect(rect.adjusted(0, -1, 0, -1), 4, 5);
                  p.drawRoundedRect(rect.adjusted(2, 1, -2, -2), 4, 5);
                  }

                  {
                  QLinearGradient lg(QPoint(0, 0), QPoint(w, 0));
                  lg.setColorAt(0.0, dark);
                  lg.setColorAt(0.1, Qt::transparent);
                  lg.setColorAt(0.9, Qt::transparent);
                  lg.setColorAt(1.0, dark);
                  p.setPen(QPen(lg, 1));
                  p.drawRoundedRect(rect.adjusted(1, 0, -1, -2), 4, 5);
                  }

            p.restore();
            // top and bottom border
            drawSeparator(&p, QRect(0, 0, w, 2), color, Qt::Horizontal);
            drawSeparator(&p, QRect(0, h - 2, w, 2), color, Qt::Horizontal);
            p.end();
            tileSet = new TileSet(QPixmap::fromImage(pm), 4, 4, w - 8, h - 8);
            m_dockFrameCache.insert(key, tileSet);
            }
      return tileSet;
      }

//---------------------------------------------------------
//   windecoButton
//---------------------------------------------------------

QPixmap StyleHelper::windecoButton(const QColor& color, bool pressed, int size) const {
      const quint64 key((quint64(color.rgba()) << 32) | (size << 1) | quint64(pressed));
      QPixmap* pixmap = m_windecoButtonCache.object(key);

      if (!pixmap) {
            QImage image(size, size,  QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::transparent);

            const QColor light(calcLightColor(color));
            const QColor dark(calcDarkColor(color));

            QPainter p(&image);
            p.setRenderHints(QPainter::Antialiasing);
            p.setPen(Qt::NoPen);
            const qreal u(size / 18.0 );
            p.translate(0.5 * u, (0.5 - 0.668) * u);

                  {
                  // outline circle
                  qreal penWidth = 1.2;
                  QLinearGradient lg( 0, u * (1.665 - penWidth), 0, u * (12.33 + 1.665 - penWidth) );
                  lg.setColorAt( 0, dark );
                  lg.setColorAt( 1, light );
                  QRectF r( u * 0.5 * (17 - 12.33 + penWidth), u * (1.665 + penWidth), u * (12.33 - penWidth), u * (12.33 - penWidth) );
                  p.setPen( QPen( lg, penWidth * u ) );
                  p.drawEllipse( r );
                  p.end();
                  }
            pixmap = new QPixmap(QPixmap::fromImage(image));
            m_windecoButtonCache.insert(key, pixmap);
            }
      return *pixmap;
      }

