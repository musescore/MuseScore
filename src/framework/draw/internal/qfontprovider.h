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
#pragma once

#include <QHash>

#include "../ifontprovider.h"

namespace muse::draw {
class FontEngineFT;
class QFontProvider : public IFontProvider
{
public:
    QFontProvider() = default;

    int addSymbolFont(const String& family, const io::path_t& path) override;

    double lineSpacing(const Font& f) const override;
    double xHeight(const Font& f) const override;
    double height(const Font& f) const override;
    double capHeight(const Font& f) const override;
    double ascent(const Font& f) const override;
    double descent(const Font& f) const override;

    bool inFont(const Font& f, Char ch) const override;
    bool inFontUcs4(const Font& f, char32_t ucs4) const override;

    // Text
    double horizontalAdvance(const Font& f, const String& string) const override;
    double horizontalAdvance(const Font& f, const Char& ch) const override;

    RectF boundingRect(const Font& f, const String& string) const override;
    RectF boundingRect(const Font& f, const Char& ch) const override;
    RectF boundingRect(const Font& f, const RectF& r, int flags, const String& string) const override;
    RectF tightBoundingRect(const Font& f, const String& string) const override;

    // Score symbols
    RectF symBBox(const Font& f, char32_t ucs4, double DPI_F) const override;
    double symAdvance(const Font& f, char32_t ucs4, double DPI_F) const override;

private:

    FontEngineFT* symEngine(const Font& f) const;

    QHash<QString /*family*/, io::path_t> m_symbolsFonts;
    mutable QHash<QString /*path*/, FontEngineFT*> m_symEngines;
};
}
