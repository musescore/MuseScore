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

#pragma once

#include "types/string.h"

namespace mu::notation {
class SearchCommandsParser
{
public:
    struct SearchData
    {
        enum class Type : unsigned char {
            Invalid,
            RehearsalMark,
            Measure,
            MeasureRange,
            Page
        };

        Type type = Type::Invalid;
        std::variant<std::monostate, muse::String, size_t, std::pair<size_t, size_t> > value;

        bool isValid() const
        {
            return type != Type::Invalid;
        }

        static SearchData makeRehearsalMark(const muse::String& name)
        {
            SearchData data;
            data.type = Type::RehearsalMark;
            data.value = name;
            return data;
        }

        muse::String rehearsalMark() const
        {
            assert(type == Type::RehearsalMark);
            return std::get<muse::String>(value);
        }

        static SearchData makeMeasure(size_t index)
        {
            SearchData data;
            data.type = Type::Measure;
            data.value = index;
            return data;
        }

        int measureIndex() const
        {
            assert(type == Type::Measure);
            return static_cast<int>(std::get<size_t>(value));
        }

        static SearchData makeMeasureRange(size_t startIndex, size_t endIndex)
        {
            SearchData data;
            data.type = Type::MeasureRange;
            data.value = std::make_pair(startIndex, endIndex);
            return data;
        }

        std::pair<size_t, size_t> measureRange() const
        {
            assert(type == Type::MeasureRange);
            return std::get<std::pair<size_t, size_t> >(value);
        }

        static SearchData makePage(size_t index)
        {
            SearchData data;
            data.type = Type::Page;
            data.value = index;
            return data;
        }

        size_t pageIndex() const
        {
            assert(type == Type::Page);
            return std::get<size_t>(value);
        }
    };

    static SearchData parse(const QString& searchCommand);

private:
    static SearchData parseMeasureCommand(const QString& searchCommand);
    static SearchData parsePageCommand(const QString& searchCommand);
    static SearchData parseRehearsalMarkCommand(const QString& searchCommand);
};
}
