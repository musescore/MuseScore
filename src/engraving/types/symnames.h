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
#ifndef MU_ENGRAVING_SYMNAMES_H
#define MU_ENGRAVING_SYMNAMES_H

#include <vector>
#include <array>
#include <unordered_map>
#include <QString>
#include "symid.h"
#include "types/string.h"

namespace mu::engraving {
struct SymNames {
    static AsciiString nameForSymId(SymId id);
    static const char* userNameForSymId(SymId id);
    static QString translatedUserNameForSymId(SymId id);

    static SymId symIdByName(const AsciiString& name, SymId def = SymId::noSym);
    static SymId symIdByName(const QString& name, SymId def = SymId::noSym);
    static SymId symIdByOldName(const QString& oldName);
    static SymId symIdByUserName(const QString& userName);

private:
    static void loadNameToSymIdHash();

    static const std::array<AsciiString, size_t(SymId::lastSym) + 1> s_symNames;
    static const std::array<const char*, size_t(SymId::lastSym) + 1> s_symUserNames;

    //! Will be initialized when first used
    static std::map<AsciiString, SymId> s_nameToSymIdHash;
    static const std::unordered_map<QString, SymId> s_oldNameToSymIdHash;
};
}

#endif // MU_ENGRAVING_SYMNAMES_H
