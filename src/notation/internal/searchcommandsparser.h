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
#ifndef MU_NOTATION_SEARCHCOMMANDSPARSER_H
#define MU_NOTATION_SEARCHCOMMANDSPARSER_H

#include "notationtypes.h"

namespace mu::notation {
class SearchCommandsParser
{
public:
    static SearchCommands availableCommands();

    struct SearchData
    {
        ElementType elementType = ElementType::INVALID;
        QVariant value;

        bool isValid()
        {
            return elementType != ElementType::INVALID;
        }
    };

    static SearchData parse(const QString& searchCommand);

private:
    static SearchData parseMeasureCommand(const QString& searchCommand);
    static SearchData parsePageCommand(const QString& searchCommand);
    static SearchData parseRehearsalMarkCommand(const QString& searchCommand);
};
}

#endif // MU_NOTATION_SEARCHCOMMANDSPARSER_H
