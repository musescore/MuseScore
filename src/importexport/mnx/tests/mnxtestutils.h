/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <string>

#include "types/string.h"

namespace mu::engraving {
class MasterScore;
class Score;
}

namespace mu::iex::mnxio {
//! @brief Helpers shared by the MNX test translation units.
//! @details These live in the module namespace so that the fixtures, which already pull it in
//! wholesale, can call them unqualified.

//! @brief Apply the fixups a freshly read or imported score needs before it can be compared.
void fixupAndLayoutScore(engraving::MasterScore* score);

//! @brief Export a score to MNX JSON, honouring the current export settings.
//! @return The serialized document, or an empty string if the export failed.
std::string exportMnxJson(engraving::Score* score);

//! @brief Import MNX JSON into a new score, failing the test if it is not valid MNX.
//! @param json The document to import.
//! @param virtualPath The path to report the score under. It need not exist on disk.
//! @return The imported score, which the caller owns, or nullptr.
engraving::MasterScore* importMnxFromJson(const std::string& json, const muse::String& virtualPath);
} // namespace mu::iex::mnxio
