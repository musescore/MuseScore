//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
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
#include "PDFDoc.h"
#include "goo/GooString.h"
#include "OutputDev.h"
#include "GlobalParams.h"
#include "GfxState.h"
#include "Gfx.h"
#include "Object.h"

//------------------------------------------------------------------------
// QImageOutputDev
//------------------------------------------------------------------------

class QImageOutputDev: public OutputDev {
    QImage* image;
    int ny;
    
public:
    QImageOutputDev()           { image = 0;   }
    void setImage(QImage* img)  { image = img; }
    virtual ~QImageOutputDev()  {}
    
    virtual GBool interpretType3Chars() { return gFalse; }
    virtual GBool needNonText()         { return gTrue; }
    
    // 0,0 is top left corner
    virtual GBool upsideDown()          { return gFalse; }
    
    virtual GBool useDrawChar()         { return gFalse; }
    
    //----- image drawing
    virtual void drawImage(GfxState *state, Object *ref, Stream *str,
                           int width, int height, GfxImageColorMap *colorMap,
                           GBool interpolate, int *maskColors, GBool inlineImg);
//    virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
//                                 int width, int height,
//                                 GfxImageColorMap *colorMap,
//                                 GBool interpolate,
//                                 Stream *maskStr, int maskWidth, int maskHeight,
//                                 GBool maskInvert, GBool maskInterpolate);
//    virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
//                                     int width, int height,
//                                     GfxImageColorMap *colorMap,
//                                     GBool interpolate,
//                                     Stream *maskStr,
//                                     int maskWidth, int maskHeight,
//                                     GfxImageColorMap *maskColorMap,
//                                     GBool maskInterpolate);
};

//---------------------------------------------------------
//   drawImage
//---------------------------------------------------------

void QImageOutputDev::drawImage(GfxState* state, Object* , Stream* str,
                                int width, int height, GfxImageColorMap* colorMap, GBool, int*, GBool)
{
    if (colorMap->getNumPixelComps() == 1 && colorMap->getBits() == 1) {
        double* ctm = state->getCTM();
        double xmag = width/ctm[0];
        double ymag = height/ctm[3];
        double xoff = (ctm[2]+ctm[4]) * xmag;
        int ph      = state->getPageHeight() * ymag;
        int yoff    = int(ph - (ctm[3]+ctm[5]) * ymag);
        
        //yoff needs to be > 0
        if(yoff < 0)
            yoff = 0;
        
        printf("Image %4d %4d   at %5f %4d   mag %f %f\n",
               width, height, xoff, yoff, xmag, ymag);
        
        bool invertBits = colorMap->getDecodeLow(0) == 0.0;
        
        if (image->isNull()) {
#if 0
            int pw = state->getPageWidth() * xmag;
            pw     = ((pw+31)/32) * 32;
            int ph = state->getPageHeight() * ymag;
#endif
            int pw = width;
            int ph = height;
            pw     = ((pw+31)/32) * 32;
            
            *image = QImage(pw, ph, QImage::Format_MonoLSB);
            QVector<QRgb> ct(2);
            ct[0] = qRgb(255, 255, 255);
            ct[1] = qRgb(0, 0, 0);
            image->setColorTable(ct);
            image->fill(0);
            ny = yoff;
            printf("  bytes %d\n", (pw * ph) / 8);
        }
        
        // copy the stream
        str->reset();     // initialize stream
        
        if (yoff != ny)
            printf("  ***next image does not fit, gap %d\n", yoff - ny);
        yoff = ny;
        
        int stride  = (width + 7) / 8;
        //            uchar mask  = 0xff << (stride * 8 - width);
        int qstride = image->bytesPerLine();
        uchar* p    = image->bits();
        
        printf("Image stride %d qstride %d yoff %d  bytes %d\n",
               stride, qstride, yoff, height * stride);
        
        for (int y = 0; y < height; ++y) {
            p = image->scanLine(y + yoff);
            for (int x = 0; x < stride; ++x) {
                static unsigned char invert[16] = {
                    0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15
                };
                uchar c = str->getChar();
                if (invertBits)
                    c = ~c;
                *p++ = (invert[c & 15] << 4) | invert[(c >> 4) & 15];
            }
            //TODO                  p[-1] &= ~mask;
        }
        str->close();
        ny += height;
    }
    else {
        printf("Color Image ================%d %d - %d %d\n", width, height,
               colorMap->getNumPixelComps(), colorMap->getBits());
#if 0
        fprintf(f, "P6\n");
        fprintf(f, "%d %d\n", width, height);
        fprintf(f, "255\n");
        
        // initialize stream
        ImageStream* imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(), colorMap->getBits());
        imgStr->reset();
        
        // for each line...
        for (int y = 0; y < height; ++y) {
            
            // write the line
            Guchar *p = imgStr->getLine();
            for (int x = 0; x < width; ++x) {
                GfxRGB rgb;
                colorMap->getRGB(p, &rgb);
                fputc(colToByte(rgb.r), f);
                fputc(colToByte(rgb.g), f);
                fputc(colToByte(rgb.b), f);
                p += colorMap->getNumPixelComps();
            }
        }
#endif
    }
}

namespace Ms {


int Pdf::references;

//---------------------------------------------------------
//   numPages
//---------------------------------------------------------

int Pdf::numPages() const
      {
      return _doc->getNumPages();
      }

//---------------------------------------------------------
//   Pdf
//---------------------------------------------------------

Pdf::Pdf()
      {
      imgOut = 0;
      globalParams        = new GlobalParams();
      ++references;

      }

//---------------------------------------------------------
//   open
//---------------------------------------------------------

bool Pdf::open(const QString& path)
      {
      GooString* fileName = new GooString(qPrintable(path));
      _doc = new PDFDoc(fileName, 0, 0);
      return true;
      }

//---------------------------------------------------------
//   ~Pdf
//---------------------------------------------------------

Pdf::~Pdf()
      {
      if(_doc)
        delete(_doc);
      if(globalParams)
        delete(globalParams);
      --references;
      }

//---------------------------------------------------------
//   page
//---------------------------------------------------------

QImage Pdf::page(int i)
      {

      if (imgOut == 0)
            imgOut = new QImageOutputDev();
      QImage image;
      imgOut->setImage(&image);
      // useMediaBox, crop, printing
      _doc->displayPage(imgOut, i+1, 1200.0, 1200.0, 0, gTrue, gFalse, gFalse);
          /*PDFRectangle r(0,0,_doc->getPageMediaHeight(i+1), _doc->getPageCropWidth(i+1));
      GfxState *state = new GfxState(1200.0, 1200.0, &r, 0, imgOut->upsideDown());
          
          //Stream *str = Gfx::buildImageStream();
          GfxColorSpace *colorSpace = new GfxDeviceGrayColorSpace();
          //Dict *dict = str->getDict();
          //Object obj1, obj2;
          // get size
          //dict->lookup("Width", &obj1);
          //dict->lookup("Height", &obj2);
          Object obj;
          obj.initNull();
          //_doc->getDocInfo(&obj);
          GfxImageColorMap *colorMap = new GfxImageColorMap(1, &obj, colorSpace);
          imgOut->drawImage(state, NULL, _doc->getBaseStream(), 50, 50, colorMap, gTrue, NULL, gTrue);*/
      /*fz_page* page = fz_load_page(ctx, doc, i);
      if (page == 0) {
            printf("cannot load page %d\n", i);
            return QImage();
            }
      static const float resolution = 600.0;
      const float zoom = resolution / 72.0;
      fz_rect bounds;

      fz_bound_page(ctx, page, &bounds);
      fz_matrix ctm;

      fz_pre_scale(fz_rotate(&ctm, 0.0), zoom, zoom);

      fz_irect ibounds;
      fz_rect tbounds;
      tbounds = bounds;
      fz_round_rect(&ibounds, fz_transform_rect(&tbounds, &ctm));
      fz_pixmap* pix = fz_new_pixmap_with_bbox(ctx, fz_device_gray(ctx), &ibounds);

      fz_clear_pixmap_with_value(ctx, pix, 255);
      fz_device* dev = fz_new_draw_device(ctx, pix);
      fz_run_page(ctx, page, dev, &ctm, NULL);
      fz_drop_device(ctx,dev);
      dev = NULL;

      int w = fz_pixmap_width(ctx, pix);
      int h = fz_pixmap_height(ctx, pix);
      if (fz_pixmap_components(ctx, pix) != 2) {
            printf("omg: pixmap not bw? %d\n", fz_pixmap_components(ctx, pix));
            return QImage();
            }

      printf("page %d  %d x %d\n", i, w, h);
      QImage image(w, h, QImage::Format_MonoLSB);
      QVector<QRgb> ct(2);
      ct[0] = qRgb(255, 255, 255);
      ct[1] = qRgb(0, 0, 0);
      image.setColorTable(ct);

      uchar* s   = fz_pixmap_samples(ctx, pix);
      int bytes  = w / 8;
      int bits   = w % 8;
      for (int line = 0; line < h; ++line) {
            uchar* d = image.scanLine(line);
            for (int col = 0; col < bytes; ++col) {
                  uchar data = 0;
                  for (int i = 0; i < 8; ++i) {
                        uchar v = *s;
                        s += 2;
                        data >>= 1;
                        if (v < 128) {            // convert grayscale to bw
                              data |= 0x80;
                              }
                        }
                  *d++ = data;
                  }
            uchar data = 0;
            for (int col = 0; col < bits; ++col) {
                  uchar v = *s;
                  s += 2;
                  data >>= 1;
                  if (v < 128)
                        data |= 0x80;
                  }
            }
      fz_drop_pixmap(ctx, pix);*/
      return image;
      }
}

