/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "infrastructure/draw/geometry.h"

namespace Ms {
class MStyle;
class XmlReader;
}

namespace mu::engraving::compat {
//---------------------------------------------------------
//   @@ PageFormat
//---------------------------------------------------------

class PageFormat
{
    mu::SizeF _size;
    qreal _printableWidth;          // _width - left margin - right margin
    qreal _evenLeftMargin;          // values in inch
    qreal _oddLeftMargin;
    qreal _evenTopMargin;
    qreal _evenBottomMargin;
    qreal _oddTopMargin;
    qreal _oddBottomMargin;
    bool _twosided;

public:
    PageFormat() {}

    const SizeF& size() const { return _size; } // size in inch
    qreal width() const { return _size.width(); }
    qreal height() const { return _size.height(); }
    void setSize(const SizeF& s) { _size = s; }

    void read206(Ms::XmlReader&);

    qreal evenLeftMargin() const { return _evenLeftMargin; }
    qreal oddLeftMargin() const { return _oddLeftMargin; }
    qreal evenTopMargin() const { return _evenTopMargin; }
    qreal evenBottomMargin() const { return _evenBottomMargin; }
    qreal oddTopMargin() const { return _oddTopMargin; }
    qreal oddBottomMargin() const { return _oddBottomMargin; }
    qreal printableWidth() const { return _printableWidth; }

    void setEvenLeftMargin(qreal val) { _evenLeftMargin = val; }
    void setOddLeftMargin(qreal val) { _oddLeftMargin = val; }
    void setEvenTopMargin(qreal val) { _evenTopMargin = val; }
    void setEvenBottomMargin(qreal val) { _evenBottomMargin = val; }
    void setOddTopMargin(qreal val) { _oddTopMargin = val; }
    void setOddBottomMargin(qreal val) { _oddBottomMargin = val; }
    void setPrintableWidth(qreal val) { _printableWidth = val; }

    bool twosided() const { return _twosided; }
    void setTwosided(bool val) { _twosided = val; }

    // convenience functions
    qreal evenRightMargin() const { return _size.width() - _printableWidth - _evenLeftMargin; }
    qreal oddRightMargin() const { return _size.width() - _printableWidth - _oddLeftMargin; }
};

void readPageFormat206(Ms::MStyle* style, Ms::XmlReader& e);
}
#endif // MU_ENGRAVING_READPAGEFORMAT_H
