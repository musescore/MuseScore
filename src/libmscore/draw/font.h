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
    Font(const QString& family);
    Font(const Font& font);

    QString family() const;

    qreal pointSizeF() const;
    void setPointSizeF(qreal s);

    bool bold() const;
    void setBold(bool arg);

private:

    QString m_family;
    qreal m_pointSizeF = 0;
    bool m_bold = false;
};
}

#endif // MU_DRAW_FONT_H
