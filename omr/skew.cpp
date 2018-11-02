//=============================================================================
//  MusE Reader
//  Music Score Reader
//
//  Copyright (C) 2010 Werner Schweer
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

#include "image.h"
#include "utils.h"
#include "omr.h"
#include "omrpage.h"

namespace Ms {

//=============================================================================
//  inspired by ImageMagick (http://www.imagemagick.org)
//=============================================================================

//  imageSkew() calculates skew of image.  Skew is an artifact that
//  occurs in scanned images because of the camera being misaligned,
//  imperfections in the scanning or surface, or simply because the paper was
//  not placed completely flat when scanned.
//
//  The format of the imageSkew method is:
//
//      double imageSkew(const QImage *image)
//

// Benchmark 7.3sec test2 first page
//           6
//           5.7
//           5.3
//           3.74
//           3.30
//           0.47sec RELEASE

#define MyPI  3.14159265358979323846264338327950288419716939937510

//---------------------------------------------------------
//   RadiansToDegrees
//---------------------------------------------------------

static inline double RadiansToDegrees(double r) {
      return 180.0 * r / MyPI;
      }

//---------------------------------------------------------
//   RadonInfo
//---------------------------------------------------------

class RadonInfo {
      int size;

   public:
      ushort* cells;
      int width, height;

      RadonInfo(ulong w, ulong h) {
            width  = w;
            height = h;
            size   = w * h;
            cells  = new ushort[size];
            }
      ~RadonInfo() { delete[] cells; }
      void reset() { memset(cells, 0, size * sizeof(*cells)); }
      ushort getCell(int x, int y) const         { return cells[height * x + y];  }
      void   setCell(int x, int y, ushort value) { cells[height * x + y] = value; }
      };

//---------------------------------------------------------
//   radonProjection
//---------------------------------------------------------

static void radonProjection(RadonInfo* src, RadonInfo* dst, int sign, ulong* projection)
      {
      RadonInfo* p = src;
      RadonInfo* q = dst;
      for (int step = 1; step < p->width; step *= 2) {
            for (int x = 0; x < p->width; x += 2 * step) {
                  for (int i = 0; i < step; i++) {
                        int y;
                        for (y = 0; y < (p->height-i-1); y++) {
                              ushort cell = p->getCell(x+i, y);
                              q->setCell(x+2*i,   y, cell + p->getCell(x+i+step, y+i));
                              q->setCell(x+2*i+1, y, cell + p->getCell(x+i+step, y+i+1));
                              }
                        for ( ; y < (p->height-i); y++) {
                              ushort cell = p->getCell(x+i, y);
                              q->setCell(x+2*i, y, cell + p->getCell(x+i+step, y+i));
                              q->setCell(x+2*i+1, y, cell);
                              }
                        for ( ; y < p->height; y++) {
                              ushort cell = p->getCell(x+i, y);
                              q->setCell(x+2*i, y, cell);
                              q->setCell(x+2*i+1, y, cell);
                              }
                        }
                  }
            RadonInfo* swap = p;
            p = q;
            q = swap;
            }
      for (int x = 0; x < p->width; x++) {
            uint sum = 0;
            for (int y = 0; y < (p->height-1); y++) {
                  int delta = p->getCell(x, y) - p->getCell(x, y + 1);
                  sum += delta * delta;
                  }
            projection[p->width + sign * x - 1] = sum;
            }
      }

//---------------------------------------------------------
//   radonTransform
//---------------------------------------------------------

void OmrPage::radonTransform(ulong* projection, int w, int n, const QRect& r)
      {
      int h = r.height();
      RadonInfo* src = new RadonInfo(w, h);
      RadonInfo* dst = new RadonInfo(w, h);

      src->reset();
      for (int y = 0; y < h; y++) {
            int i = n;
            const uchar* p = (const uchar*)scanLine(r.y() + y);
            for (int x = 0; x < n; ++x)
                  src->setCell(--i, y, Omr::bitsSetTable[*p++]);
            }
      radonProjection(src, dst, -1, projection);

      src->reset();
      for (int y = 0; y < h; y++) {
            const uchar* p = (const uchar*)scanLine(r.y() + y);
            for (int x = 0; x < n; ++x)
                  src->setCell(x, y, Omr::bitsSetTable[*p++]);
            }
      radonProjection(src, dst, 1, projection);

      delete dst;
      delete src;
      }

//---------------------------------------------------------
//   skew
//    compute image skew angle
//---------------------------------------------------------

double OmrPage::skew(const QRect& r)
      {
//      Benchmark bench("imageSkew");

      int nn    = wordsPerLine() * 4;
      int width = 1;
      for (; width < nn; width <<= 1)
            ;
      int n = 2 * width - 1;

      ulong* projection = new ulong[n];
      radonTransform(projection, width, nn, r);
      uint max_projection = 0;
      int skew            = 0;
      for (int i = 0; i < n; i++) {
            if (projection[i] > max_projection) {
                  skew = i - width + 1;
                  max_projection = projection[i];
                  }
            }
      delete[] projection;
      return RadiansToDegrees(-atan((double) skew/width/8));
      }
}

