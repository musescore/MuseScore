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
#ifndef MU_NOTATION_NOTATIONMETAWRITER_H
#define MU_NOTATION_NOTATIONMETAWRITER_H

#include "abstractnotationwriter.h"

namespace Ms {
class Score;
}

namespace mu::notation {
class NotationMetaWriter : public AbstractNotationWriter
{
public:
    Ret write(notation::INotationPtr notation, io::Device& destinationDevice, const Options& options = Options()) override;

private:
    std::string title(const Ms::Score* score) const;
    std::string subtitle(const Ms::Score* score) const;
    std::string composer(const Ms::Score* score) const;
    std::string poet(const Ms::Score* score) const;
    std::string timesig(const Ms::Score* score) const;
    std::pair<std::string, std::string> tempo(const Ms::Score* score) const;
    std::string parts(const Ms::Score* score) const;
    std::string pageFormat(const Ms::Score* score) const;
    std::string typeData(Ms::Score* score);
};
}

#endif // MU_NOTATION_MSCZNOTATIONMETAWRITER_H
