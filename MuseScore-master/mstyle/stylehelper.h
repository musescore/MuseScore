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

#ifndef __HELPER_H__
#define __HELPER_H__

#include "tileset.h"
#include "colorscheme.h"
#include "animationmodes.h"
#include "cache.h"

typedef BaseCache<QColor> ColorCache;
typedef BaseCache<TileSet> TileSetCache;
typedef BaseCache<QPixmap> PixmapCache;

typedef QMap<quint32, bool> ColorMap;

//---------------------------------------------------------
//   StyleHelper
//---------------------------------------------------------

class StyleHelper {
            mutable ColorCache m_midColorCache;
            mutable ColorCache m_backgroundColorCache;
            mutable ColorCache m_backgroundTopColorCache;
            mutable ColorCache m_backgroundBottomColorCache;
            mutable ColorCache m_decoColorCache;
            mutable ColorCache m_lightColorCache;
            mutable ColorCache m_darkColorCache;
            mutable ColorCache m_shadowColorCache;
            mutable ColorCache m_backgroundRadialColorCache;


            mutable TileSetCache m_cornerCache;
            mutable TileSetCache m_slabSunkenCache;
            mutable TileSetCache m_slabInvertedCache;
            mutable TileSetCache m_holeCache;
            mutable TileSetCache m_holeFlatCache;
            mutable TileSetCache m_slopeCache;
            mutable TileSetCache m_grooveCache;
            mutable TileSetCache m_slitCache;
            mutable TileSetCache m_dockFrameCache;
            mutable TileSetCache m_scrollHoleCache;
            mutable TileSetCache m_selectionCache;

            mutable Cache<QPixmap> m_dialSlabCache;
            mutable Cache<QPixmap> m_roundSlabCache;
            mutable Cache<TileSet> m_holeFocusedCache;
            mutable Cache<TileSet> m_slabCache;
            mutable PixmapCache m_backgroundCache;
            mutable PixmapCache m_progressBarCache;
            mutable PixmapCache m_windecoButtonCache;
            mutable PixmapCache m_windecoButtonGlowCache;
            mutable PixmapCache m_dotCache;

            qreal _bgcontrast;
            qreal _contrast;
            static const qreal _shadowGain;
            static const qreal _glowBias;
            static const qreal _slabThickness;

            StatefulBrush _viewHoverBrush;
            StatefulBrush _viewFocusBrush;
            StatefulBrush _viewNegativeTextBrush;

            mutable ColorMap m_highThreshold;
            mutable ColorMap m_lowThreshold;

            void drawHole(QPainter&, const QColor&, qreal shade, int r = 7) const;

      public:
            StyleHelper();
            StatefulBrush viewHoverBrush() const {
                  return _viewHoverBrush;
                  }
            StatefulBrush viewFocusBrush() const {
                  return _viewFocusBrush;
                  }

            TileSet* hole(const QColor& color, qreal shade, int size, bool outline) const;
            TileSet* holeFlat(const QColor&, qreal shade, int size = 7) const;

            void fillHole(QPainter&, const QRect&, int size = 7) const;

            //! generic hole
            void renderHole(QPainter* p, const QColor& color, const QRect& r, bool focus = false, bool hover = false,
                            TileSet::Tiles posFlags = TileSet::Ring, bool outline = false) const {
                  renderHole(p, color, r, focus, hover, -1, AnimationNone, posFlags, outline);
                  }
            //! generic hole (with animated glow)
            void renderHole(QPainter* p, const QColor&, const QRect& r,
                            bool focus, bool hover, qreal opacity, AnimationMode animationMode,
                            TileSet::Tiles posFlags = TileSet::Ring, bool outline = false) const;
            TileSet* holeFocused(const QColor&, const QColor& glowColor, qreal shade, int size = 7, bool outline = false) const;

            inline const QColor& calcMidColor(const QColor& color) const;
            const QWidget* checkAutoFillBackground( const QWidget* ) const;

            //! returns menu background color matching position in a top level widget of given height
            const QColor& backgroundColor(const QColor& color, int height, int y) const {
                  return backgroundColor(color, qMin(qreal(1.0), qreal(y) / qMin(300, 3 * height / 4)));
                  }
            const QColor& backgroundColor(const QColor&, qreal ratio) const;

            const QColor& backgroundColor(const QColor& color, const QWidget* w, const QPoint& point) const {
                  if (!(w && w->window()) || checkAutoFillBackground(w))
                        return color;
                  else
                        return backgroundColor(color, w->window()->height(), w->mapTo(w->window(), point).y());
                  }

            const QColor& backgroundRadialColor(const QColor& color) const;
            const QColor& backgroundTopColor(const QColor& color) const;
            const QColor& backgroundBottomColor(const QColor& color) const;

            bool lowThreshold(const QColor& color) const;
            bool highThreshold(const QColor& color) const;
            static QColor alphaColor(QColor color, qreal alpha);
            const QColor& calcLightColor(const QColor& color) const;
            const QColor& calcDarkColor(const QColor& color) const;
            void drawInverseShadow(QPainter&, const QColor&, int pad, int size, qreal fuzz) const;
            void drawInverseGlow(QPainter&, const QColor&, int pad, int size, int rsize) const;
            const QColor& calcShadowColor(const QColor& color) const;
            void renderMenuBackground(QPainter* p, const QRect& clipRect, const QWidget* widget, const QPalette& pal) const {
                  renderMenuBackground(p, clipRect, widget, pal.color(widget->window()->backgroundRole()));
                  }
            // render menu background
            void renderMenuBackground(QPainter*, const QRect&, const QWidget*, const QColor&) const;
            QPixmap verticalGradient(const QColor& color, int height, int offset = 0) const;
            QPixmap radialGradient(const QColor& color, int width, int height = 64) const;
            inline bool hasAlphaChannel(const QWidget*) const;
            bool compositingActive() const {
                  return false;      // return KWindowSystem::compositingActive();
                  }
            void renderWindowBackground(QPainter* p, const QRect& clipRect, const QWidget* widget,
                                        const QPalette& pal, int y_shift = -23, int gradientHeight = 64) const {
                  renderWindowBackground( p, clipRect, widget, pal.color( widget->window()->backgroundRole() ), y_shift, gradientHeight );
                  }
            /*!
            y_shift: shift the background gradient upwards, to fit with the windec
            gradientHeight: the height of the generated gradient.
            for different heights, the gradient is translated so that it is always at the same position from the bottom
            */
            void renderWindowBackground(QPainter* p, const QRect& clipRect, const QWidget* widget,
                                        const QWidget* window, const QPalette& pal, int y_shift = -23, int gradientHeight = 64) const {
                  renderWindowBackground( p, clipRect, widget, window, pal.color( window->backgroundRole() ), y_shift, gradientHeight );
                  }

            //! render window background using a given color as a reference
            void renderWindowBackground(QPainter* p, const QRect& clipRect, const QWidget* widget,
                                        const QColor& color, int y_shift = -23, int gradientHeight = 64) const {
                  renderWindowBackground( p, clipRect, widget, widget->window(), color, y_shift, gradientHeight );
                  }

            //! render window background using a given color as a reference
            void renderWindowBackground(QPainter* p, const QRect& clipRect, const QWidget* widget,
                                        const QWidget* window, const QColor& color, int y_shift = -23, int gradientHeight = 64) const;
            void renderDot(QPainter*, const QPoint&, const QColor&) const;
            void drawSeparator(QPainter* p, const QRect& r, const QColor& color, Qt::Orientation orientation) const;
            const QColor& decoColor(const QColor& background, const QColor& color) const;
            //! returns a region matching given rect, with rounded corners, based on the multipliers
            /*! setting any of the multipliers to zero will result in no corners shown on the corresponding side */
            QRegion roundedMask( const QRect&, int left = 1, int right = 1, int top = 1, int bottom = 1 ) const;

            //!@name slabs
            //@{

            void fillSlab(QPainter&, const QRect&, int size = 7) const;

            // progressbar
            QPixmap progressBarIndicator(const QPalette&, const QRect&) const;

            QPixmap dialSlab(const QColor&, qreal shade, int size = 7) const;
            QPixmap dialSlabFocused(const QColor&, const QColor&, qreal shade, int size = 7) const;
            QPixmap roundSlab(const QColor&, qreal shade, int size = 7) const;
            QPixmap roundSlabFocused(const QColor&, const QColor& glowColor, qreal shade, int size = 7) const;

            TileSet* slabFocused(const QColor&, const QColor& glowColor, qreal shade, int size = 7) const;
            TileSet* slabSunken(const QColor&, qreal shade, int size = 7) const;
            TileSet* slabInverted(const QColor&, qreal shade, int size = 7) const;
            TileSet* slab(const QColor&, qreal shade, int size = 7) const;

            //! draw frame that mimics some sort of shadows around a panel
            /*! it is used for menus, detached dock panels and toolbar, as well as window decoration when compositing is disabled */
            void drawFloatFrame(
                  QPainter* p, const QRect r, const QColor& color,
                  bool drawUglyShadow = true, bool isActive = false,
                  const QColor& frameColor = QColor(),
                  TileSet::Tiles tiles = TileSet::Ring) const;
            TileSet* scrollHole(const QColor&, Qt::Orientation orientation, bool smallShadow = false) const;
            //! round corners (used for Menus, combobox drop-down, detached toolbars and dockwidgets
            TileSet* roundCorner(const QColor&, int size = 5) const;

            //! groupbox background
            TileSet* slope(const QColor&, qreal shade, int size = 7) const;
            TileSet* selection( const QColor&, int height, bool custom) const;
            //! focus rect for flat toolbuttons
            TileSet* slitFocused(const QColor&) const;
            void drawSlab(QPainter&, const QColor&, qreal shade) const;
            void drawShadow(QPainter&, const QColor&, int size) const;
            void drawOuterGlow(QPainter&, const QColor&, int size) const;
            void drawRoundSlab( QPainter&, const QColor&, qreal ) const;

            //! returns menu background color matching position in a given menu widget
            const QColor& menuBackgroundColor(const QColor& color, const QWidget* w, const QPoint& point) const {
                  if ( !( w && w->window() ) || checkAutoFillBackground( w ) )
                        return color;
                  else
                        return menuBackgroundColor( color, w->window()->height(), w->mapTo( w->window(), point ).y() );
                  }
            //! returns menu background color matching position in a menu widget of given height
            const QColor& menuBackgroundColor(const QColor& color, int height, int y) const {
                  return backgroundColor( color, qMin(qreal(1.0), qreal(y) / qMin(200, 3 * height / 4) ) );
                  }
            //! scrollbar groove
            TileSet* groove(const QColor&, qreal shade, int size = 7) const;
            QPalette mergePalettes( const QPalette&, qreal ratio ) const;
            TileSet* dockFrame(const QColor&, int size);
            QPixmap windecoButton(const QColor& color, bool pressed, int size = 21) const;
            //! negative text brush (used for close button hover)
            const StatefulBrush& viewNegativeTextBrush( void ) const {
                  return _viewNegativeTextBrush;
                  }
            void invalidateCaches();
            void setMaxCacheSize(int value);
            void reloadConfig();
      };

//---------------------------------------------------------
//   calcMidColor
//---------------------------------------------------------

const QColor& StyleHelper::calcMidColor(const QColor& color) const {
      const quint64 key(color.rgba());
      QColor* out = m_midColorCache.object(key);
      if (!out) {
            out = new QColor(ColorScheme::shade(color, ColorScheme::MidShade, _contrast - 1.0));
            m_midColorCache.insert(key, out);
            }
      return *out;
      }

//---------------------------------------------------------
//   hasAlphaChannel
//---------------------------------------------------------

bool StyleHelper::hasAlphaChannel( const QWidget* widget ) const {
#if 0
//#ifdef Q_OS_LINUX
      if (compositingActive()) {
            if (widget)
                  return widget->x11Info().depth() == 32;
            else
                  return QX11Info().appDepth() == 32;
            }
      else
            return false;
#else
      Q_UNUSED(widget)
      return compositingActive();
#endif
      }


#endif



