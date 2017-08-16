//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __READ206_H__
#define __READ206_H__

namespace Ms {

class MStyle;

//---------------------------------------------------------
//   @@ PageFormat
//---------------------------------------------------------

class PageFormat {
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
      PageFormat() {}

      const QSizeF& size() const          { return _size;          }    // size in inch
      qreal width() const                 { return _size.width();  }
      qreal height() const                { return _size.height(); }
      void setSize(const QSizeF& s)       { _size = s;             }

      void read(XmlReader&);
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
      };

extern void readArticulation(Articulation*, XmlReader&);
extern void setPageFormat(MStyle*, const PageFormat&);
extern void initPageFormat(MStyle*, PageFormat*);
}

#endif


