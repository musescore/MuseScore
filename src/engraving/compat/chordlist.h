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
#ifndef MU_ENGRAVING_CHORDLIST_H
#define MU_ENGRAVING_CHORDLIST_H

#include <functional>
#include <QString>

namespace Ms {
class XmlReader;
class XmlWriter;
class Score;
}

namespace mu::engraving::compat {
struct ReadChordListHook
{
    ReadChordListHook(Ms::Score* score);

    void read(Ms::XmlReader& e);
    void validate();

private:
    Ms::Score* m_score = nullptr;
    bool m_chordListTag = false;
    QString m_oldChordDescriptionFile;
};

struct WriteChordListHook
{
    WriteChordListHook(Ms::Score* score);

    void write(Ms::XmlWriter& xml);

private:
    Ms::Score* m_score = nullptr;
};
}

#endif // MU_ENGRAVING_CHORDLIST_H
