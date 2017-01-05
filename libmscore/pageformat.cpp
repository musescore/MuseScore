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

#include "pageformat.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

PageFormat::PageFormat()
      {
      _size             = QSizeF(210.0/INCH, 297.0/INCH); // A4
      _evenLeftMargin   = 10.0 / INCH;
      _oddLeftMargin    = 10.0 / INCH;
      _printableWidth   = _size.width() - 20.0 / INCH;
      _evenTopMargin    = 10.0 / INCH;
      _evenBottomMargin = 20.0 / INCH;
      _oddTopMargin     = 10.0 / INCH;
      _oddBottomMargin  = 20.0 / INCH;
      _twosided         = true;
      }

//---------------------------------------------------------
//   read
//  <page-layout>
//      <page-height>
//      <page-width>
//      <landscape>1</landscape>
//      <page-margins type="both">
//         <left-margin>28.3465</left-margin>
//         <right-margin>28.3465</right-margin>
//         <top-margin>28.3465</top-margin>
//         <bottom-margin>56.6929</bottom-margin>
//         </page-margins>
//      </page-layout>
//---------------------------------------------------------

void PageFormat::read(XmlReader& e)
      {
      qreal _oddRightMargin  = 0.0;
      qreal _evenRightMargin = 0.0;
      QString type;

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "page-margins") {
                  type = e.attribute("type","both");
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        qreal val = e.readDouble() * 0.5 / PPI;
                        if (tag == "left-margin")
                              lm = val;
                        else if (tag == "right-margin")
                              rm = val;
                        else if (tag == "top-margin")
                              tm = val;
                        else if (tag == "bottom-margin")
                              bm = val;
                        else
                              e.unknown();
                        }
                  _twosided = type == "odd" || type == "even";
                  if (type == "odd" || type == "both") {
                        _oddLeftMargin   = lm;
                        _oddRightMargin  = rm;
                        _oddTopMargin    = tm;
                        _oddBottomMargin = bm;
                        }
                  if (type == "even" || type == "both") {
                        _evenLeftMargin   = lm;
                        _evenRightMargin  = rm;
                        _evenTopMargin    = tm;
                        _evenBottomMargin = bm;
                        }
                  }
            else if (tag == "page-height")
                  _size.rheight() = e.readDouble() * 0.5 / PPI;
            else if (tag == "page-width")
                  _size.rwidth() = e.readDouble() * .5 / PPI;
            else
                  e.unknown();
            }
      qreal w1        = _size.width() - _oddLeftMargin - _oddRightMargin;
      qreal w2        = _size.width() - _evenLeftMargin - _evenRightMargin;
      _printableWidth = qMin(w1, w2);     // silently adjust right margins
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void PageFormat::write(XmlWriter& xml) const
      {
      xml.stag("page-layout");

      // convert inch to 1/10 spatium units
      // 20 - font design size in point
      // SPATIUM = 20/4
      // qreal t = 10 * PPI / (20 / 4);
      qreal t = 2 * PPI;

      xml.tag("page-height", _size.height() * t);
      xml.tag("page-width",  _size.width() * t);

      const char* type = "both";
      if (_twosided) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   evenLeftMargin() * t);
            xml.tag("right-margin",  evenRightMargin() * t);
            xml.tag("top-margin",    evenTopMargin() * t);
            xml.tag("bottom-margin", evenBottomMargin() * t);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   oddLeftMargin() * t);
      xml.tag("right-margin",  oddRightMargin() * t);
      xml.tag("top-margin",    oddTopMargin() * t);
      xml.tag("bottom-margin", oddBottomMargin() * t);
      xml.etag();

      xml.etag();
      }
}

