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

using namespace muse;
using namespace muse::draw;

bool Font::g_disableFontMerging = false;

Font::Font(const FontFamily& family, Type type)
    : m_family(family), m_type(type)
{
}

bool Font::operator ==(const Font& other) const
{
    //! NOTE At the moment, the type is entered for information,
    //! its correct installation is not guaranteed,
    //! so we do not take it when comparing it yet
    // && m_type == other.m_type

    return m_family == other.m_family
           && RealIsEqual(m_pointSizeF, other.m_pointSizeF)
           && m_weight == other.m_weight
           && m_style == other.m_style
           && m_noFontMerging == other.m_noFontMerging
           && m_hinting == other.m_hinting;
}

void Font::setFamily(const FontFamily& family, Type type)
{
    m_family = family;
    m_type = type;
}

Font::FontFamily Font::family() const
{
    return m_family;
}

Font::Type Font::type() const
{
    return m_type;
}

double Font::pointSizeF() const
{
    return m_pointSizeF;
}

void Font::setPointSizeF(double s)
{
    m_pointSizeF = s;
    m_pixelSize = -1;
}

int Font::pixelSize() const
{
    return m_pixelSize;
}

void Font::setPixelSize(int s)
{
    m_pixelSize = s;
    m_pointSizeF = -1.0;
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
    if (g_disableFontMerging) {
        return true;
    }
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
    QFont qf(family().id());

    if (pointSizeF() > 0) {
        qf.setPointSizeF(pointSizeF());
    } else if (pixelSize() > 0) {
        qf.setPixelSize(pixelSize());
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

Font Font::fromQFont(const QFont& qf, Font::Type type)
{
    Font f(String::fromQString(qf.family()), type);
    if (qf.pointSizeF() > 0) {
        f.setPointSizeF(qf.pointSizeF());
    } else if (qf.pixelSize() > 0) {
        f.setPixelSize(qf.pixelSize());
    }
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
