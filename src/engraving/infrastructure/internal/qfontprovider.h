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
#ifndef MU_DRAW_QFONTPROVIDER_H
#define MU_DRAW_QFONTPROVIDER_H

#include <QHash>
#include "infrastructure/draw/ifontprovider.h"

namespace mu::draw {
class FontEngineFT;
class QFontProvider : public IFontProvider
{
public:
    QFontProvider() = default;

    int addApplicationFont(const QString& family, const QString& path) override;
    void insertSubstitution(const QString& familyName, const QString& substituteName) override;

    qreal lineSpacing(const Font& f) const override;
    qreal xHeight(const Font& f) const override;
    qreal height(const Font& f) const override;
    qreal ascent(const Font& f) const override;
    qreal descent(const Font& f) const override;

    bool inFont(const Font& f, QChar ch) const override;
    bool inFontUcs4(const Font& f, uint ucs4) const override;

    // Text
    qreal horizontalAdvance(const Font& f, const QString& string) const override;
    qreal horizontalAdvance(const Font& f, const QChar& ch) const override;

    RectF boundingRect(const Font& f, const QString& string) const override;
    RectF boundingRect(const Font& f, const QChar& ch) const override;
    RectF boundingRect(const Font& f, const RectF& r, int flags, const QString& string) const override;
    RectF tightBoundingRect(const Font& f, const QString& string) const override;

    // Score symbols
    RectF symBBox(const Font& f, uint ucs4, qreal DPI_F) const override;
    qreal symAdvance(const Font& f, uint ucs4, qreal DPI_F) const override;

private:

    FontEngineFT* symEngine(const Font& f) const;

    QHash<QString /*family*/, QString /*path*/> m_paths;
    mutable QHash<QString /*path*/, FontEngineFT*> m_symEngines;
};
}

#endif // MU_DRAW_QFONTPROVIDER_H
