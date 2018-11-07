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

#include "pdf.h"

namespace Ms {


int Pdf::references;

//---------------------------------------------------------
//   numPages
//---------------------------------------------------------

int Pdf::numPages() const
      {
      return _document->numPages();
      }

//---------------------------------------------------------
//   Pdf
//---------------------------------------------------------

Pdf::Pdf()
      {
      ++references;
      }

//---------------------------------------------------------
//   open
//---------------------------------------------------------

bool Pdf::open(const QString& path)
      {
      _document = Poppler::Document::load(path);
      if (!_document || _document->isLocked()) {
            delete _document;
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   ~Pdf
//---------------------------------------------------------

Pdf::~Pdf()
      {
      if(_document)
        delete(_document);
      --references;
      }
      
//---------------------------------------------------------
//   binarization
//---------------------------------------------------------

QImage Pdf::binarization(QImage image){
      QImage bw = QImage(image.width(), image.height(), QImage::Format_MonoLSB);
      QVector<QRgb> ct(2);
      ct[0] = qRgb(255, 255, 255);
      ct[1] = qRgb(0, 0, 0);
      bw.setColorTable(ct);
      bw.fill(0);
      float thresh = 128;
      float new_thresh = 0;
      
      while (thresh != new_thresh) {
            float sum_black = 0;
            float sum_white = 0;
            int num_black = 0;
            int num_white = 0;
            new_thresh = thresh;
            for (int x = 0; x < image.width(); x++){
                  for (int y = 0; y < image.height(); y++) {
                        QRgb c = image.pixel(x, y);
                        float g = qGray(c);
                        if (g < thresh) {
                              sum_black += g;
                              num_black++;
                              }
                        else {
                              sum_white += g;
                              num_white++;
                              }
                        
                        }
                  }
            thresh = (sum_black/num_black + sum_white/num_white)/2.0;
            }
      
      int stride  = (bw.width() + 7) / 8;
      uchar* p    = bw.bits();
        
      for (int y = 0; y < bw.height(); ++y) {
            p = bw.scanLine(y);
            for (int x = 0; x < stride; ++x) {
                  int temp = 0;
                  for (int i = 0; i < 8; i++) {
                        if (x*8 + i >= bw.width()) continue;
                        QRgb c = image.pixel(x*8 + i, y);
                        float g = qGray(c);
                        temp += ((g<thresh) ? 1:0)<<i;
                        }
                  *p++ = temp;
                  }
            }
      return bw;
}

//---------------------------------------------------------
//   page
//---------------------------------------------------------

QImage Pdf::page(int i)
      {
      QImage image;
      // Paranoid safety check
      if (_document == 0) {
            return image;
            }
      
      Poppler::Page* pdfPage = _document->page(i);  // Document starts at page 0
      if (pdfPage == 0) {
            return image;
            }
            
      QSize size = pdfPage->pageSize();
      float scale = 2.0;
      // the size can be decided more intelligently
      image = pdfPage->renderToImage(scale*72.0, scale*72.0, 0, 0, scale*size.width(), scale*size.height());
      delete pdfPage;
      return binarization(image);
      }
}

