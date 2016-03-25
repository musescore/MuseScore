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

#include <math.h>
#include "colorutils.h"

//---------------------------------------------------------
//   ColorUtils
//---------------------------------------------------------

static inline qreal wrap(qreal a, qreal d = 1.0) {
      qreal r = fmod(a, d);
      return (r < 0.0 ? d + r : (r > 0.0 ? r : 0.0));
      }

//---------------------------------------------------------
//   KHCY
//---------------------------------------------------------

KHCY::KHCY(qreal h_, qreal c_, qreal y_, qreal a_) {
      h = h_;
      c = c_;
      y = y_;
      a = a_;
      }

KHCY::KHCY(const QColor& color) {
      qreal r = gamma(color.redF());
      qreal g = gamma(color.greenF());
      qreal b = gamma(color.blueF());
      a = color.alphaF();

      // luma component
      y = lumag(r, g, b);

      // hue component
      qreal p = qMax(qMax(r, g), b);
      qreal n = qMin(qMin(r, g), b);
      qreal d = 6.0 * (p - n);
      if (n == p)
            h = 0.0;
      else if (r == p)
            h = ((g - b) / d);
      else if (g == p)
            h = ((b - r) / d) + (1.0 / 3.0);
      else
            h = ((r - g) / d) + (2.0 / 3.0);

      // chroma component
      if (r == g && g == b)
            c = 0.0;
      else
            c = qMax( (y - n) / y, (p - y) / (1 - y) );
      }

//---------------------------------------------------------
//   qColor
//---------------------------------------------------------

QColor KHCY::qColor() const {
      // start with sane component values
      qreal _h = wrap(h);
      qreal _c = normalize(c);
      qreal _y = normalize(y);

      // calculate some needed variables
      qreal _hs = _h * 6.0, th, tm;
      if (_hs < 1.0) {
            th = _hs;
            tm = yc[0] + yc[1] * th;
            }
      else if (_hs < 2.0) {
            th = 2.0 - _hs;
            tm = yc[1] + yc[0] * th;
            }
      else if (_hs < 3.0) {
            th = _hs - 2.0;
            tm = yc[1] + yc[2] * th;
            }
      else if (_hs < 4.0) {
            th = 4.0 - _hs;
            tm = yc[2] + yc[1] * th;
            }
      else if (_hs < 5.0) {
            th = _hs - 4.0;
            tm = yc[2] + yc[0] * th;
            }
      else {
            th = 6.0 - _hs;
            tm = yc[0] + yc[2] * th;
            }

      // calculate RGB channels in sorted order
      qreal tn, to, tp;
      if (tm >= _y) {
            tp = _y + _y * _c * (1.0 - tm) / tm;
            to = _y + _y * _c * (th - tm) / tm;
            tn = _y - (_y * _c);
            }
      else {
            tp = _y + (1.0 - _y) * _c;
            to = _y + (1.0 - _y) * _c * (th - tm) / (1.0 - tm);
            tn = _y - (1.0 - _y) * _c * tm / (1.0 - tm);
            }

      // return RGB channels in appropriate order
      if (_hs < 1.0)
            return QColor::fromRgbF(igamma(tp), igamma(to), igamma(tn), a);
      else if (_hs < 2.0)
            return QColor::fromRgbF(igamma(to), igamma(tp), igamma(tn), a);
      else if (_hs < 3.0)
            return QColor::fromRgbF(igamma(tn), igamma(tp), igamma(to), a);
      else if (_hs < 4.0)
            return QColor::fromRgbF(igamma(tn), igamma(to), igamma(tp), a);
      else if (_hs < 5.0)
            return QColor::fromRgbF(igamma(to), igamma(tn), igamma(tp), a);
      else
            return QColor::fromRgbF(igamma(tp), igamma(tn), igamma(to), a);
      }

//---------------------------------------------------------
//   luma
//---------------------------------------------------------

qreal KHCY::luma(const QColor& color) {
      return lumag(gamma(color.redF()), gamma(color.greenF()), gamma(color.blueF()));
      }

//---------------------------------------------------------
//   mixQreal
//---------------------------------------------------------

static inline qreal mixQreal(qreal a, qreal b, qreal bias) {
      return a + (b - a) * bias;
      }

//---------------------------------------------------------
//   tintHelper
//---------------------------------------------------------

QColor ColorUtils::tintHelper(const QColor& base, const QColor& color, qreal amount) {
      KHCY result(ColorUtils::mix(base, color, pow(amount, 0.3)));
      result.y = mixQreal(ColorUtils::luma(base), result.y, amount);
      return result.qColor();
      }

//---------------------------------------------------------
//   contrastRatio
//---------------------------------------------------------

qreal ColorUtils::contrastRatio(const QColor& c1, const QColor& c2) {
      qreal y1 = luma(c1), y2 = luma(c2);
      if (y1 > y2)
            return (y1 + 0.05) / (y2 + 0.05);
      else
            return (y2 + 0.05) / (y1 + 0.05);
      }

//---------------------------------------------------------
//   tint
//---------------------------------------------------------

QColor ColorUtils::tint(const QColor& base, const QColor& color, qreal amount) {
      if (amount <= 0.0)
            return base;
      if (amount >= 1.0)
            return color;
#ifdef Q_OS_MAC
      if (isnan(amount))
#else      
      if (std::isnan(amount))
#endif
            return base;

      double ri = contrastRatio(base, color);
      double rg = 1.0 + ((ri + 1.0) * amount * amount * amount);
      double u = 1.0, l = 0.0;
      QColor result;
      for (int i = 12 ; i ; --i) {
            double a = 0.5 * (l + u);
            result = tintHelper(base, color, a);
            double ra = contrastRatio(base, result);
            if (ra > rg)
                  u = a;
            else
                  l = a;
            }
      return result;
      }

//---------------------------------------------------------
//   mix
//---------------------------------------------------------

QColor ColorUtils::mix(const QColor& c1, const QColor& c2, qreal bias) {
      if (bias <= 0.0)
            return c1;
      if (bias >= 1.0)
            return c2;
#ifdef Q_OS_MAC
      if (isnan(bias))
#else      
      if (std::isnan(bias))
#endif
            return c1;

      qreal r = mixQreal(c1.redF(),   c2.redF(),   bias);
      qreal g = mixQreal(c1.greenF(), c2.greenF(), bias);
      qreal b = mixQreal(c1.blueF(),  c2.blueF(),  bias);
      qreal a = mixQreal(c1.alphaF(), c2.alphaF(), bias);

      return QColor::fromRgbF(r, g, b, a);
      }

//---------------------------------------------------------
//   lighten
//---------------------------------------------------------

QColor ColorUtils::lighten(const QColor& color, qreal ky, qreal kc) {
      KHCY c(color);
      c.y = 1.0 - normalize((1.0 - c.y) * (1.0 - ky));
      c.c = 1.0 - normalize((1.0 - c.c) * kc);
      return c.qColor();
      }

//---------------------------------------------------------
//   darken
//---------------------------------------------------------

QColor ColorUtils::darken(const QColor& color, qreal ky, qreal kc) {
      KHCY c(color);
      c.y = normalize(c.y * (1.0 - ky));
      c.c = normalize(c.c * kc);
      return c.qColor();
      }

//---------------------------------------------------------
//   shade
//---------------------------------------------------------

QColor ColorUtils::shade(const QColor& color, qreal ky, qreal kc) {
      KHCY c(color);
      c.y = normalize(c.y + ky);
      c.c = normalize(c.c + kc);
      return c.qColor();
      }


