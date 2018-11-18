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

#include "ocr.h"

#include <tesseract/baseapi.h>
#include <locale.h>

namespace Ms {

//---------------------------------------------------------
//   Ocr
//---------------------------------------------------------

Ocr::Ocr()
      {
      tess = 0;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Ocr::init()
      {
      if (tess == 0)
            tess = new tesseract::TessBaseAPI;
      tess->Init("/usr/local/share/tessdata", 0, 0, 0, false);
      }

//---------------------------------------------------------
//   readLine
//---------------------------------------------------------

QString Ocr::readLine(const OcrImage& img)
      {
      int w = img.r.width();
      int h = img.r.height();

      int bw = (w + 7) / 8;
      uchar* d = new uchar[bw * h];
      memset(d, 0, bw * h);
      uchar* p = d;
      int yo = img.r.y();
      int xo = img.r.x();

      for (int y = 0; y < h; ++y) {
            for (int x = 0; x < bw; ++x) {
                  int mask = 0x80;
                  uchar dst = 0;
                  for (int xx = 0; xx < 8; ++xx) {
                        if (img.dot(x * 8 + xo + xx, y + yo))
                              dst |= mask;
                        mask >>= 1;
                        }
                  *p++ = ~dst;
                  }
            }
      char* txt = tess->TesseractRect(d, 0, bw, 0, 0, w, h);
      return QString(txt);
      }
}

