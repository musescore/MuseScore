/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#ifndef MU_CONVERTER_NOTATIONMETA_H
#define MU_CONVERTER_NOTATIONMETA_H

#include "notation/inotation.h"

namespace mu::engraving {
class Score;
class MStyle;
}

namespace mu::converter {
class NotationMeta
{
public:
    static muse::RetVal<std::string> metaJson(notation::INotationPtr notation);

private:
    static QString title(const mu::engraving::Score* score);
    static QString subtitle(const mu::engraving::Score* score);
    static QString composer(const mu::engraving::Score* score);
    static QString poet(const mu::engraving::Score* score);
    static QString timesig(const mu::engraving::Score* score);
    static std::pair<int, QString> tempo(const mu::engraving::Score* score);
    static QJsonArray partsJsonArray(const mu::engraving::Score* score);
    static QJsonObject pageFormatJson(const mu::engraving::MStyle& style);
    static QJsonObject typeDataJson(mu::engraving::Score* score);
};
}

#endif // MU_CONVERTER_NOTATIONMETA_H
