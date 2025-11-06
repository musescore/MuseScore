/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include "fontproviderdispatcher.h"

#include "fontprovider.h"
#include "qfontprovider.h"

#include "log.h"

using namespace muse;
using namespace muse::draw;

FontProviderDispatcher::FontProviderDispatcher(std::shared_ptr<FontProvider> mainFProvider,
                                               std::shared_ptr<QFontProvider> qtFProvider)
    : m_mainFProvider(mainFProvider), m_qtFProvider(qtFProvider)
{
}

int FontProviderDispatcher::addSymbolFont(const muse::String& family, const io::path_t& path)
{
    //! NOTE Need only for QFontProvider
    return m_qtFProvider->addSymbolFont(family, path);
}

double FontProviderDispatcher::lineSpacing(const muse::draw::Font& f) const
{
    return m_mainFProvider->lineSpacing(f);
}

double FontProviderDispatcher::xHeight(const muse::draw::Font& f) const
{
    return m_mainFProvider->xHeight(f);
}

double FontProviderDispatcher::height(const muse::draw::Font& f) const
{
    return m_mainFProvider->height(f);
}

double FontProviderDispatcher::ascent(const muse::draw::Font& f) const
{
    return m_mainFProvider->ascent(f);
}

double FontProviderDispatcher::capHeight(const muse::draw::Font& f) const
{
    return m_mainFProvider->capHeight(f);
}

double FontProviderDispatcher::descent(const muse::draw::Font& f) const
{
    return m_mainFProvider->descent(f);
}

bool FontProviderDispatcher::inFont(const muse::draw::Font& f, char32_t ucs4) const
{
    bool ret = m_mainFProvider->inFont(f, ucs4);
    return ret;
}

// Text
double FontProviderDispatcher::horizontalAdvance(const muse::draw::Font& f, const muse::String& string) const
{
    return m_qtFProvider->horizontalAdvance(f, string);
}

double FontProviderDispatcher::horizontalAdvance(const muse::draw::Font& f, char32_t ucs4) const
{
    return m_qtFProvider->horizontalAdvance(f, ucs4);
}

RectF FontProviderDispatcher::boundingRect(const muse::draw::Font& f, const muse::String& string) const
{
    return m_qtFProvider->boundingRect(f, string);
}

RectF FontProviderDispatcher::boundingRect(const muse::draw::Font& f, char32_t ucs4) const
{
    return m_qtFProvider->boundingRect(f, ucs4);
}

RectF FontProviderDispatcher::tightBoundingRect(const muse::draw::Font& f, const muse::String& string) const
{
    return m_qtFProvider->tightBoundingRect(f, string);
}
