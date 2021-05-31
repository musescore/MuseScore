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
    Font(const Font& font);

    enum class Style {
        Undefined = 0,
        Bold = 1 << 1,
        Italic = 1 << 2,
        Underline = 1 << 3
    };

    void setFamily(const QString& family);
    QString family() const;

    qreal pointSizeF() const;
    void setPointSizeF(qreal s);

    bool bold() const;
    void setBold(bool arg);
    bool italic() const;
    void setItalic(bool arg);
    bool underline() const;
    void setUnderline(bool arg);

    void setNoFontMerging(bool arg);
    bool noFontMerging() const;

private:

    QString m_family;
    qreal m_pointSizeF = 0;
    QFlags<Style> m_style{ Style::Undefined };
    bool m_noFontMerging = false;
};
}

#endif // MU_DRAW_FONT_H
