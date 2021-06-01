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
#include "fontcompat.h"

QFont mu::draw::toQFont(const Font& f)
{
    QFont qf(f.family());
    qf.setPointSizeF(f.pointSizeF());
    qf.setWeight(static_cast<QFont::Weight>(f.weight()));
    qf.setBold(f.bold());
    qf.setItalic(f.italic());
    qf.setUnderline(f.underline());
    if (f.noFontMerging()) {
        qf.setStyleStrategy(QFont::NoFontMerging);
    }
    qf.setHintingPreference(static_cast<QFont::HintingPreference>(f.hinting()));

    return qf;
}

mu::draw::Font mu::draw::fromQFont(const QFont& qf)
{
    mu::draw::Font f(qf.family());
    f.setPointSizeF(qf.pointSizeF());
    f.setWeight(static_cast<Font::Weight>(qf.weight()));
    f.setBold(qf.bold());
    f.setItalic(qf.italic());
    f.setUnderline(qf.underline());
    if (qf.styleStrategy() == QFont::NoFontMerging) {
        f.setNoFontMerging(true);
    }
    f.setHinting(static_cast<Font::Hinting>(qf.hintingPreference()));
    return f;
}
