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

#ifndef __OCR_H__
#define __OCR_H__


namespace tesseract {
      class TessBaseAPI;
      };

namespace Ms {


//---------------------------------------------------------
//   OcrImage
//---------------------------------------------------------

struct OcrImage {
      uint* image;
      QRect r;
      int stride; // uint* stride

      OcrImage() {}
      OcrImage(const uchar* p, const QRect& _r, int _s) : image((uint*)p), r(_r), stride(_s) {}
      OcrImage crop() const;
      bool dot(int x, int y) const {
            return (*(image + (y * stride) + (x / 32))) & (0x1 << (x % 32));
            }
      };

//---------------------------------------------------------
//   Ocr
//---------------------------------------------------------

class Ocr {
      tesseract::TessBaseAPI* tess;

   public:
      Ocr();
      void init();
      QString readLine(const OcrImage&);
      };
}

#endif

