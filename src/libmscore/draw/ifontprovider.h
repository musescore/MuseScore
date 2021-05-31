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

#ifndef MU_ENGRAVING_DRAWIFONTPROVIDER_H
#define MU_ENGRAVING_DRAWIFONTPROVIDER_H

#include "modularity/imoduleexport.h"

#include "font.h"

namespace mu::draw {
class IFontProvider : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::draw::IFontProvider)

public:
    virtual ~IFontProvider() = default;

    virtual qreal lineSpacing(const Font& f) const = 0;
    virtual qreal xHeight(const Font& f) const = 0;
    virtual qreal height(const Font& f) const = 0;
    virtual qreal ascent(const Font& f) const = 0;
    virtual qreal descent(const Font& f) const = 0;

    virtual QRectF boundingRect(const Font& f, const QString& string) const = 0;
    virtual QRectF boundingRect(const Font& f, const QChar& ch) const = 0;
    virtual QRectF boundingRect(const Font& f, const QRectF& r, int flags, const QString& string) const = 0;
    virtual QRectF tightBoundingRect(const Font& f, const QString& string) const = 0;

    virtual bool inFont(const Font& f, QChar ch) const = 0;
    virtual bool inFontUcs4(const Font& f, uint ucs4) const = 0;
};
}

#endif // MU_ENGRAVING_DRAWIFONTPROVIDER_H
