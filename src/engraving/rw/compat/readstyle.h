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

#ifndef MU_ENGRAVING_READSTYLE_H
#define MU_ENGRAVING_READSTYLE_H

#include <QByteArray>
#include <QString>

namespace Ms {
class Score;
class XmlReader;
class MStyle;
}

namespace mu::engraving::compat {
class ReadStyleHook
{
public:
    ReadStyleHook(Ms::Score* score, const QByteArray& scoreData, const QString& completeBaseName);

    void setupDefaultStyle();

    void readStyleTag(Ms::XmlReader& e);

    static int styleDefaultByMscVersion(const int mscVer);
    static void setupDefaultStyle(Ms::Score* score);
    static void readStyleTag(Ms::Score* score, Ms::XmlReader& e);
    static bool readStyleProperties(Ms::MStyle* style, Ms::XmlReader& e);

private:
    Ms::Score* m_score = nullptr;
    const QByteArray& m_scoreData;
    const QString& m_completeBaseName;
};
}

#endif // MU_ENGRAVING_READSTYLE_H
