
#ifndef __COLORUTILS_H__
#define __COLORUTILS_H__

#include <math.h>

static const qreal yc[3] = {0.2126, 0.7152, 0.0722};

// normalize: like qBound(a, 0.0, 1.0) but without needing the args and with
// "safer" behavior on NaN (isnan(a) -> return 0.0)
static inline qreal normalize(qreal a) {
      return (a < 1.0 ? (a > 0.0 ? a : 0.0) : 1.0);
      }


class KHCY {
            static qreal gamma(qreal n) {
                  return pow(normalize(n), 2.2);
                  }
            static qreal igamma(qreal n)  {
                  return pow(normalize(n), 1.0 / 2.2);
                  }
            static qreal lumag(qreal r, qreal g, qreal b) {
                  return r * yc[0] + g * yc[1] + b * yc[2];
                  }

      public:
            KHCY(const QColor&);
            KHCY(qreal h_, qreal c_, qreal y_, qreal a_ = 1.0);
            QColor qColor() const;
            qreal h, c, y, a;
            static qreal luma(const QColor&);
      };


class ColorUtils {
            static QColor tintHelper(const QColor& base, const QColor& color, qreal amount);
            static qreal contrastRatio(const QColor& c1, const QColor& c2);
      public:
            static QColor tint(const QColor&, const QColor&, qreal amount = .3);
            static QColor mix(const QColor&, const QColor&, qreal bias = .5);
            static qreal luma(const QColor& color) {
                  return KHCY::luma(color);
                  }
            static QColor shade(const QColor&, qreal lumaAmount, qreal chromaAmount = 0.0);
            static QColor darken(const QColor&, qreal amount = 0.5, qreal chromaGain = 1.0);
            static QColor lighten(const QColor&, qreal amount = 0.5, qreal chromaInverseGain = 1.0);
      };

#endif

