/*
 * Copyright 2009-2010 Hugo Pereira Da Costa <hugo@oxygen-icons.org>
 * Copyright 2008 Long Huynh Huu <long.upcase@googlemail.com>
 * Copyright 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "tileset.h"

//---------------------------------------------------------
//   initPixmap
//---------------------------------------------------------

void TileSet::initPixmap( PixmapList& pixmaps, const QPixmap& pix, int w, int h, const QRect& rect) {
      QSize size( w, h );
      if (!size.isValid()) {
            pixmaps.push_back(QPixmap());
            }
      else if (size != rect.size()) {
            const QPixmap tile(pix.copy(rect));
            QPixmap pixmap(w, h);

            pixmap.fill(Qt::transparent);
            QPainter p(&pixmap);
            p.drawTiledPixmap(QRect(0, 0, w, h), tile);

            pixmaps.push_back( pixmap );
            }
      else
            pixmaps.push_back(pix.copy(rect));
      }

//---------------------------------------------------------
//   TileSet
//---------------------------------------------------------

TileSet::TileSet()
      : _w1(0), _h1(0), _w3(0), _h3(0) {
      _pixmaps.reserve(9);
      }

TileSet::TileSet(const QPixmap& pix, int w1, int h1, int w2, int h2)
      : _w1(w1), _h1(h1), _w3(0), _h3(0) {
      _pixmaps.reserve(9);
      if (pix.isNull())
            return;

      _w3 = pix.width() - (w1 + w2);
      _h3 = pix.height() - (h1 + h2);
      int w = w2;
      while (w < 32 && w2 > 0)
            w += w2;
      int h = h2;
      while (h < 32 && h2 > 0)
            h += h2;

      // initialise pixmap array
      initPixmap(_pixmaps, pix, _w1, _h1, QRect(0, 0, _w1, _h1));
      initPixmap(_pixmaps, pix, w, _h1, QRect(_w1, 0, w2, _h1));
      initPixmap(_pixmaps, pix, _w3, _h1, QRect(_w1 + w2, 0, _w3, _h1));
      initPixmap(_pixmaps, pix, _w1, h, QRect(0, _h1, _w1, h2));
      initPixmap(_pixmaps, pix, w, h, QRect(_w1, _h1, w2, h2));
      initPixmap(_pixmaps, pix, _w3, h, QRect(_w1 + w2, _h1, _w3, h2));
      initPixmap(_pixmaps, pix, _w1, _h3, QRect(0, _h1 + h2, _w1, _h3));
      initPixmap(_pixmaps, pix, w, _h3, QRect(_w1, _h1 + h2, w2, _h3));
      initPixmap(_pixmaps, pix, _w3, _h3, QRect(_w1 + w2, _h1 + h2, _w3, _h3));
      }

TileSet::TileSet(const QPixmap& pix, int w1, int h1, int w3, int h3, int x1, int y1, int w2, int h2)
      : _w1(w1), _h1(h1), _w3(w3), _h3(h3) {
      _pixmaps.reserve(9);
      if (pix.isNull())
            return;

      int x2 = pix.width() - _w3;
      int y2 = pix.height() - _h3;
      int w = w2;
      while (w < 32 && w2 > 0) w += w2;
      int h = h2;
      while (h < 32 && h2 > 0) h += h2;

      // initialise pixmap array
      initPixmap(_pixmaps, pix, _w1, _h1, QRect(0, 0, _w1, _h1));
      initPixmap(_pixmaps, pix, w, _h1, QRect(x1, 0, w2, _h1));
      initPixmap(_pixmaps, pix, _w3, _h1, QRect(x2, 0, _w3, _h1));
      initPixmap(_pixmaps, pix, _w1, h, QRect(0, y1, _w1, h2));
      initPixmap(_pixmaps, pix, w, h, QRect(x1, y1, w2, h2));
      initPixmap(_pixmaps, pix, _w3, h, QRect(x2, y1, _w3, h2));
      initPixmap(_pixmaps, pix, _w1, _h3, QRect(0, y2, _w1, _h3));
      initPixmap(_pixmaps, pix, w, _h3, QRect(x1, y2, w2, _h3));
      initPixmap(_pixmaps, pix, _w3, _h3, QRect(x2, y2, _w3, _h3));
      }

//---------------------------------------------------------
//   bits
//---------------------------------------------------------

inline bool bits(TileSet::Tiles flags, TileSet::Tiles testFlags) {
      return (flags & testFlags) == testFlags;
      }

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void TileSet::render(const QRect& r, QPainter* p, Tiles t) const
      {
      // check initialization
      if (_pixmaps.size() < 9)
            return;

      int x0, y0, w, h;
      r.getRect(&x0, &y0, &w, &h);

      // calculate pixmaps widths
      int wLeft(0);
      int wRight(0);
      if (_w1 + _w3 > 0) {
            qreal wRatio(qreal(_w1) / qreal(_w1 + _w3));
            wLeft = (t & Right) ? qMin(_w1, int(w * wRatio)) : _w1;
            wRight = (t & Left) ? qMin(_w3, int(w * (1.0 - wRatio))) : _w3;
            }

      // calculate pixmap heights
      int hTop(0);
      int hBottom(0);
      if (_h1 + _h3 > 0) {
            qreal hRatio( qreal( _h1 ) / qreal( _h1 + _h3 ) );
            hTop = (t & Bottom) ? qMin( _h1, int(h * hRatio) ) : _h1;
            hBottom = (t & Top) ? qMin( _h3, int(h * (1.0 - hRatio)) ) : _h3;
            }

      // calculate corner locations
      w -= wLeft + wRight;
      h -= hTop + hBottom;
      int x1 = x0 + wLeft;
      int x2 = x1 + w;
      int y1 = y0 + hTop;
      int y2 = y1 + h;

      // corner
      if (bits(t, Top | Left))
            p->drawPixmap(x0, y0, _pixmaps.at(0), 0, 0, wLeft, hTop);
      if (bits(t, Top | Right))
            p->drawPixmap(x2, y0, _pixmaps.at(2), _w3 - wRight, 0, wRight, hTop);
      if (bits(t, Bottom | Left))
            p->drawPixmap(x0, y2, _pixmaps.at(6), 0, _h3 - hBottom, wLeft,  hBottom);
      if (bits(t, Bottom | Right))
            p->drawPixmap(x2, y2, _pixmaps.at(8), _w3 - wRight, _h3 - hBottom, wRight, hBottom);

      // top and bottom
      if (w > 0) {
            if (t & Top)
                  p->drawTiledPixmap(x1, y0, w, hTop, _pixmaps.at(1));
            if (t & Bottom)
                  p->drawTiledPixmap(x1, y2, w, hBottom, _pixmaps.at(7), 0, _h3 - hBottom);
            }

      // left and right
      if (h > 0) {
            if (t & Left)
                  p->drawTiledPixmap(x0, y1, wLeft, h, _pixmaps.at(3));
            if (t & Right)
                  p->drawTiledPixmap(x2, y1, wRight, h, _pixmaps.at(5), _w3 - wRight, 0);
            }

      // center
      if ((t & Center) && h > 0 && w > 0)
            p->drawTiledPixmap(x1, y1, w, h, _pixmaps.at(4));
      }

