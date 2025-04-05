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
#ifndef MU_ENGRAVING_READPAGEFORMAT_H
#define MU_ENGRAVING_READPAGEFORMAT_H

#include "draw/types/geometry.h"

namespace mu::engraving {
class MStyle;
class XmlReader;
}

namespace mu::engraving::compat {
//---------------------------------------------------------
//   @@ PageFormat
//---------------------------------------------------------

class PageFormat
{
    muse::SizeF _size;
    double _printableWidth;          // _width - left margin - right margin
    double _evenLeftMargin;          // values in inch
    double _oddLeftMargin;
    double _evenTopMargin;
    double _evenBottomMargin;
    double _oddTopMargin;
    double _oddBottomMargin;
    bool _twosided;

public:
    PageFormat() {}

    const muse::SizeF& size() const { return _size; } // size in inch
    double width() const { return _size.width(); }
    double height() const { return _size.height(); }
    void setSize(const muse::SizeF& s) { _size = s; }

    void read206(XmlReader&);

    double evenLeftMargin() const { return _evenLeftMargin; }
    double oddLeftMargin() const { return _oddLeftMargin; }
    double evenTopMargin() const { return _evenTopMargin; }
    double evenBottomMargin() const { return _evenBottomMargin; }
    double oddTopMargin() const { return _oddTopMargin; }
    double oddBottomMargin() const { return _oddBottomMargin; }
    double printableWidth() const { return _printableWidth; }

    void setEvenLeftMargin(double val) { _evenLeftMargin = val; }
    void setOddLeftMargin(double val) { _oddLeftMargin = val; }
    void setEvenTopMargin(double val) { _evenTopMargin = val; }
    void setEvenBottomMargin(double val) { _evenBottomMargin = val; }
    void setOddTopMargin(double val) { _oddTopMargin = val; }
    void setOddBottomMargin(double val) { _oddBottomMargin = val; }
    void setPrintableWidth(double val) { _printableWidth = val; }

    bool twosided() const { return _twosided; }
    void setTwosided(bool val) { _twosided = val; }

    // convenience functions
    double evenRightMargin() const { return _size.width() - _printableWidth - _evenLeftMargin; }
    double oddRightMargin() const { return _size.width() - _printableWidth - _oddLeftMargin; }
};

void initPageFormat(MStyle* style, PageFormat* pf);
void setPageFormat(MStyle* style, const PageFormat& pf);
void readPageFormat206(MStyle* style, XmlReader& e);
}
#endif // MU_ENGRAVING_READPAGEFORMAT_H
