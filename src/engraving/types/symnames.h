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
#ifndef MU_ENGRAVING_SYMNAMES_H
#define MU_ENGRAVING_SYMNAMES_H

#include <vector>
#include <array>
#include <unordered_map>

#include "types/string.h"
#include "types/translatablestring.h"

#include "symid.h"

namespace mu::engraving {
struct SymNames {
    static muse::AsciiStringView nameForSymId(SymId id);
    static const muse::TranslatableString& userNameForSymId(SymId id);
    static muse::String translatedUserNameForSymId(SymId id);

    static SymId symIdByName(const muse::AsciiStringView& name, SymId def = SymId::noSym);
    static SymId symIdByName(const muse::String& name, SymId def = SymId::noSym);
    static SymId symIdByOldName(const muse::AsciiStringView& oldName);
    static SymId symIdByUserName(const muse::String& userName);

private:
    static void loadNameToSymIdHash();

    static const std::array<muse::AsciiStringView, size_t(SymId::lastSym) + 1> s_symNames;
    static const std::array<muse::TranslatableString, size_t(SymId::lastSym) + 1> s_symUserNames;

    //! Will be initialized when first used
    static std::map<muse::AsciiStringView, SymId> s_nameToSymIdHash;
    static const std::map<muse::AsciiStringView, SymId> s_oldNameToSymIdHash;
};
}

#endif // MU_ENGRAVING_SYMNAMES_H
