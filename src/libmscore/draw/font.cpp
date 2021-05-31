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
#include "font.h"

using namespace mu::draw;

Font::Font(const QString& family)
    : m_family(family)
{
}

Font::Font(const Font& f)
    : m_family(f.m_family), m_pointSizeF(f.m_pointSizeF), m_bold(f.m_bold)
{
}

QString Font::family() const
{
    return m_family;
}

qreal Font::pointSizeF() const
{
    return m_pointSizeF;
}

void Font::setPointSizeF(qreal s)
{
    m_pointSizeF = s;
}

bool Font::bold() const
{
    return m_bold;
}

void Font::setBold(bool arg)
{
    m_bold = arg;
}
