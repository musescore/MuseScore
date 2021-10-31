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
#include "global/realfn.h"

using namespace mu::draw;

Font::Font(const QString& family)
    : m_family(family)
{
}

bool Font::operator ==(const Font& other) const
{
    return m_family == other.m_family
           && RealIsEqual(m_pointSizeF, other.m_pointSizeF)
           && m_weight == other.m_weight
           && m_style == other.m_style
           && m_noFontMerging == other.m_noFontMerging
           && m_hinting == other.m_hinting;
}

void Font::setFamily(const QString& family)
{
    m_family = family;
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

Font::Weight Font::weight() const
{
    return m_weight;
}

void Font::setWeight(Weight w)
{
    m_weight = w;
}

bool Font::bold() const
{
    return m_style.testFlag(Style::Bold);
}

void Font::setBold(bool arg)
{
    m_style.setFlag(Style::Bold, arg);
}

bool Font::italic() const
{
    return m_style.testFlag(Style::Italic);
}

void Font::setItalic(bool arg)
{
    m_style.setFlag(Style::Italic, arg);
}

bool Font::underline() const
{
    return m_style.testFlag(Style::Underline);
}

void Font::setUnderline(bool arg)
{
    m_style.setFlag(Style::Underline, arg);
}

bool Font::strike() const
{
    return m_style.testFlag(Style::Strike);
}

void Font::setStrike(bool arg)
{
    m_style.setFlag(Style::Strike, arg);
}

void Font::setNoFontMerging(bool arg)
{
    m_noFontMerging = arg;
}

bool Font::noFontMerging() const
{
    return m_noFontMerging;
}

Font::Hinting Font::hinting() const
{
    return m_hinting;
}

void Font::setHinting(Hinting hinting)
{
    m_hinting = hinting;
}

#ifndef NO_QT_SUPPORT
QFont Font::toQFont() const
{
    QFont qf(family());

    if (pointSizeF() > 0) {
        qf.setPointSizeF(pointSizeF());
    }
    qf.setWeight(static_cast<QFont::Weight>(weight()));
    qf.setBold(bold());
    qf.setItalic(italic());
    qf.setUnderline(underline());
    qf.setStrikeOut(strike());
    if (noFontMerging()) {
        qf.setStyleStrategy(QFont::NoFontMerging);
    }
    qf.setHintingPreference(static_cast<QFont::HintingPreference>(hinting()));

    return qf;
}

Font Font::fromQFont(const QFont& qf)
{
    mu::draw::Font f(qf.family());
    f.setPointSizeF(qf.pointSizeF());
    f.setWeight(static_cast<Font::Weight>(qf.weight()));
    f.setBold(qf.bold());
    f.setItalic(qf.italic());
    f.setUnderline(qf.underline());
    f.setStrike(qf.strikeOut());
    if (qf.styleStrategy() == QFont::NoFontMerging) {
        f.setNoFontMerging(true);
    }
    f.setHinting(static_cast<Font::Hinting>(qf.hintingPreference()));
    return f;
}

#endif
