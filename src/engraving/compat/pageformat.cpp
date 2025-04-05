/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "pageformat.h"

#include "style/style.h"
#include "rw/xmlreader.h"

#include "dom/mscore.h"

namespace mu::engraving::compat {
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

void PageFormat::read206(XmlReader& e)
{
    double _oddRightMargin  = 0.0;
    double _evenRightMargin = 0.0;
    AsciiStringView type;

    while (e.readNextStartElement()) {
        const AsciiStringView tag(e.name());
        if (tag == "page-margins") {
            type = e.asciiAttribute("type", "both");
            double lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
            while (e.readNextStartElement()) {
                const AsciiStringView t(e.name());
                double val = e.readDouble() * 0.5 / PPI;
                if (t == "left-margin") {
                    lm = val;
                } else if (t == "right-margin") {
                    rm = val;
                } else if (t == "top-margin") {
                    tm = val;
                } else if (t == "bottom-margin") {
                    bm = val;
                } else {
                    e.unknown();
                }
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
        } else if (tag == "page-height") {
            _size.setHeight(e.readDouble() * 0.5 / PPI);
        } else if (tag == "page-width") {
            _size.setWidth(e.readDouble() * .5 / PPI);
        } else {
            e.unknown();
        }
    }
    double w1 = _size.width() - _oddLeftMargin - _oddRightMargin;
    double w2 = _size.width() - _evenLeftMargin - _evenRightMargin;
    _printableWidth = std::min(w1, w2);       // silently adjust right margins
}

void initPageFormat(MStyle* style, PageFormat* pf)
{
    SizeF sz;
    sz.setWidth(style->value(Sid::pageWidth).toReal());
    sz.setHeight(style->value(Sid::pageHeight).toReal());
    pf->setSize(sz);
    pf->setPrintableWidth(style->value(Sid::pagePrintableWidth).toReal());
    pf->setEvenLeftMargin(style->value(Sid::pageEvenLeftMargin).toReal());
    pf->setOddLeftMargin(style->value(Sid::pageOddLeftMargin).toReal());
    pf->setEvenTopMargin(style->value(Sid::pageEvenTopMargin).toReal());
    pf->setEvenBottomMargin(style->value(Sid::pageEvenBottomMargin).toReal());
    pf->setOddTopMargin(style->value(Sid::pageOddTopMargin).toReal());
    pf->setOddBottomMargin(style->value(Sid::pageOddBottomMargin).toReal());
    pf->setTwosided(style->value(Sid::pageTwosided).toBool());
}

void setPageFormat(MStyle* style, const PageFormat& pf)
{
    style->set(Sid::pageWidth,            pf.size().width());
    style->set(Sid::pageHeight,           pf.size().height());
    style->set(Sid::pagePrintableWidth,   pf.printableWidth());
    style->set(Sid::pageEvenLeftMargin,   pf.evenLeftMargin());
    style->set(Sid::pageOddLeftMargin,    pf.oddLeftMargin());
    style->set(Sid::pageEvenTopMargin,    pf.evenTopMargin());
    style->set(Sid::pageEvenBottomMargin, pf.evenBottomMargin());
    style->set(Sid::pageOddTopMargin,     pf.oddTopMargin());
    style->set(Sid::pageOddBottomMargin,  pf.oddBottomMargin());
    style->set(Sid::pageTwosided,         pf.twosided());
}

void readPageFormat206(MStyle* style, XmlReader& e)
{
    PageFormat pf;
    initPageFormat(style, &pf);
    pf.read206(e);
    setPageFormat(style, pf);
}
}
