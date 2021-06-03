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
#ifndef MU_DRAW_FONT_H
#define MU_DRAW_FONT_H

#include <QString>

namespace mu::draw {
class Font
{
public:
    Font(const QString& family = QString());

    enum class Style {
        Undefined   = 0,
        Bold        = 1 << 0,
        Italic      = 1 << 1,
        Underline   = 1 << 2
    };

    enum class Hinting {
        PreferDefaultHinting = 0,
        PreferNoHinting = 1,
        PreferVerticalHinting = 2,
        PreferFullHinting = 3
    };

    enum Weight {
        Thin     = 0,    // 100
        ExtraLight = 12, // 200
        Light    = 25,   // 300
        Normal   = 50,   // 400
        Medium   = 57,   // 500
        DemiBold = 63,   // 600
        Bold     = 75,   // 700
        ExtraBold = 81,  // 800
        Black    = 87    // 900
    };

    void setFamily(const QString& family);
    QString family() const;

    qreal pointSizeF() const;
    void setPointSizeF(qreal s);

    Weight weight() const;
    void setWeight(Weight w);

    bool bold() const;
    void setBold(bool arg);
    bool italic() const;
    void setItalic(bool arg);
    bool underline() const;
    void setUnderline(bool arg);

    void setNoFontMerging(bool arg);
    bool noFontMerging() const;

    Hinting hinting() const;
    void setHinting(Hinting hinting);

    bool operator ==(const Font& other) const;
    bool operator !=(const Font& other) const { return !this->operator ==(other); }

private:

    QString m_family;
    qreal m_pointSizeF = -1.0;
    Weight m_weight = Weight::Normal;
    QFlags<Style> m_style{ Style::Undefined };
    bool m_noFontMerging = false;
    Hinting m_hinting = Hinting::PreferDefaultHinting;
};
}

#endif // MU_DRAW_FONT_H
