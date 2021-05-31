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
#ifndef MU_ENGRAVING_DRAWQFONTPROVIDER_H
#define MU_ENGRAVING_DRAWQFONTPROVIDER_H

#include "ifontprovider.h"

namespace mu::draw {
class QFontProvider : public IFontProvider
{
public:
    QFontProvider() = default;

    qreal lineSpacing(const Font& f) const override;
    qreal xHeight(const Font& f) const override;
    qreal height(const Font& f) const override;
    qreal ascent(const Font& f) const override;
    qreal descent(const Font& f) const override;

    QRectF boundingRect(const Font& f, const QString& string) const override;
    QRectF boundingRect(const Font& f, const QChar& ch) const override;
    QRectF boundingRect(const Font& f, const QRectF& r, int flags, const QString& string) const override;
    QRectF tightBoundingRect(const Font& f, const QString& string) const override;

    bool inFont(const Font& f, QChar ch) const override;
    bool inFontUcs4(const Font& f, uint ucs4) const override;
};
}

#endif // MU_ENGRAVING_DRAWQFONTPROVIDER_H
