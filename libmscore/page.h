//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __PAGE_H__
#define __PAGE_H__

#include "config.h"
#include "element.h"
#include "bsp.h"

namespace Ms {

class System;
class Text;
class Measure;
class Xml;
class Score;
class MeasureBase;

//---------------------------------------------------------
//   PaperSize
//---------------------------------------------------------

struct PaperSize {
      const char* name;
      qreal w, h;            // size in inch
      PaperSize(const char* n, qreal wi, qreal hi)
         : name(n), w(wi), h(hi) {}
      };

extern const PaperSize* getPaperSize(const QString&);
extern const PaperSize* getPaperSize(const qreal wi, const qreal hi);

//---------------------------------------------------------
//   @@ PageFormat
//   @P size            QSizeF     paper size in inch
//   @P printableWidth   qreal
//   @P evenLeftMargin   qreal
//   @P oddLeftMargin    qreal
//   @P eventTopMargin   qreal
//   @P oddTopMargin     qreal
//   @P evenBottomMargin qreal
//   @P oddBottomMargin  qreal
//   @P twosided         bool
//---------------------------------------------------------

class PageFormat : public QObject {
      Q_OBJECT
      Q_PROPERTY(QSizeF size             READ size             WRITE setSize)
      Q_PROPERTY(qreal  printableWidth   READ printableWidth   WRITE setPrintableWidth  )
      Q_PROPERTY(qreal  evenLeftMargin   READ evenLeftMargin   WRITE setEvenLeftMargin  )
      Q_PROPERTY(qreal  oddLeftMargin    READ oddLeftMargin    WRITE setOddLeftMargin   )
      Q_PROPERTY(qreal  evenTopMargin    READ evenTopMargin    WRITE setEvenTopMargin  )
      Q_PROPERTY(qreal  oddTopMargin     READ oddTopMargin     WRITE setOddTopMargin    )
      Q_PROPERTY(qreal  evenBottomMargin READ evenBottomMargin WRITE setEvenBottomMargin)
      Q_PROPERTY(qreal  oddBottomMargin  READ oddBottomMargin  WRITE setOddBottomMargin )
      Q_PROPERTY(bool   twosided         READ twosided         WRITE setTwosided        )

      QSizeF _size;
      qreal _printableWidth;        // _width - left margin - right margin
      qreal _evenLeftMargin;        // values in inch
      qreal _oddLeftMargin;
      qreal _evenTopMargin;
      qreal _evenBottomMargin;
      qreal _oddTopMargin;
      qreal _oddBottomMargin;
      bool _twosided;

   public:
      PageFormat();

      const QSizeF& size() const    { return _size;          }    // size in inch
      qreal width() const           { return _size.width();  }
      qreal height() const          { return _size.height(); }
      void setSize(const QSizeF& s) { _size = s;             }
      void copy(const PageFormat&);

      QString name() const;
      void read(XmlReader&);
      void write(Xml&) const;
      qreal evenLeftMargin() const        { return _evenLeftMargin;   }
      qreal oddLeftMargin() const         { return _oddLeftMargin;    }
      qreal evenTopMargin() const         { return _evenTopMargin;    }
      qreal evenBottomMargin() const      { return _evenBottomMargin; }
      qreal oddTopMargin() const          { return _oddTopMargin;     }
      qreal oddBottomMargin() const       { return _oddBottomMargin;  }
      qreal printableWidth() const        { return _printableWidth;   }

      void setEvenLeftMargin(qreal val)   { _evenLeftMargin = val;   }
      void setOddLeftMargin(qreal val)    { _oddLeftMargin = val;    }
      void setEvenTopMargin(qreal val)    { _evenTopMargin = val;    }
      void setEvenBottomMargin(qreal val) { _evenBottomMargin = val; }
      void setOddTopMargin(qreal val)     { _oddTopMargin = val;     }
      void setOddBottomMargin(qreal val)  { _oddBottomMargin = val;  }
      void setPrintableWidth(qreal val)   { _printableWidth = val;   }

      bool twosided() const               { return _twosided; }
      void setTwosided(bool val)          { _twosided = val;  }

      // convenience functions
      qreal evenRightMargin() const       { return _size.width() - _printableWidth - _evenLeftMargin; }
      qreal oddRightMargin() const        { return _size.width() - _printableWidth - _oddLeftMargin;  }
      const PaperSize* paperSize() const  { return getPaperSize(_size.width(), _size.height()); }
      void setSize(const PaperSize* size);
      };

//---------------------------------------------------------
//   @@ Page
//   @P pagenumber int (read only)
//---------------------------------------------------------

class Page : public Element {
      Q_OBJECT
      Q_PROPERTY(int pagenumber READ no)

      QList<System*> _systems;
      int _no;                      // page number
#ifdef USE_BSP
      BspTree bspTree;
      void doRebuildBspTree();
#endif
      bool bspTreeValid;

      QString replaceTextMacros(const QString&) const;
      void drawStyledHeaderFooter(QPainter*, int area, const QPointF&, const QString&) const;

   public:
      Page(Score*);
      ~Page();
      virtual Page* clone() const            { return new Page(*this); }
      virtual ElementType type() const       { return PAGE; }
      const QList<System*>* systems() const  { return &_systems;   }
      QList<System*>* systems()              { return &_systems;   }

      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(XmlReader&);

      void appendSystem(System* s);

      int no() const                     { return _no;        }
      void setNo(int n);
      bool isOdd() const;
      qreal tm() const;            // margins in pixel
      qreal bm() const;
      qreal lm() const;
      qreal rm() const;

      virtual void draw(QPainter*) const;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      QList<Element*> items(const QRectF& r);
      QList<Element*> items(const QPointF& p);
      void rebuildBspTree()   { bspTreeValid = false; }
      QPointF pagePos() const { return QPointF(); }     ///< position in page coordinates
      QList<System*> searchSystem(const QPointF& pos) const;
      Measure* searchMeasure(const QPointF& p) const;
      MeasureBase* pos2measure(const QPointF&, int* staffIdx, int* pitch,
         Segment**, QPointF* offset) const;
      QList<const Element*> elements();         ///< list of visible elements
      };

extern const PaperSize paperSizes[];


}     // namespace Ms
#endif
