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
#pragma once

#include <vector>

#include "styledef.h"

namespace mu::engraving {
const std::vector<Sid>& pageStyles();

class MStyle;
class PageSizeGetAccessor
{
public:
    PageSizeGetAccessor(const MStyle& style);

    double width() const;
    double height() const;
    double printableWidth() const;
    double evenTopMargin() const;
    double evenBottomMargin() const;
    double evenLeftMargin() const;
    double oddTopMargin() const;
    double oddBottomMargin() const;
    double oddLeftMargin() const;
    double twosided() const;
    double spatium() const;

private:
    const MStyle& m_style;
};

class PageSizeSetAccessor
{
public:
    PageSizeSetAccessor(MStyle& style);

    double width() const;
    double height() const;
    double printableWidth() const;
    double evenTopMargin() const;
    double evenBottomMargin() const;
    double evenLeftMargin() const;
    double oddTopMargin() const;
    double oddBottomMargin() const;
    double oddLeftMargin() const;
    double twosided() const;
    double spatium() const;

    void setWidth(double v);
    void setHeight(double v);
    void setPrintableWidth(double v);
    void setEvenTopMargin(double v);
    void setEvenBottomMargin(double v);
    void setEvenLeftMargin(double v);
    void setOddTopMargin(double v);
    void setOddBottomMargin(double v);
    void setOddLeftMargin(double v);
    void setTwosided(double v);
    void setSpatium(double v);

private:
    MStyle& m_style;
};
}
