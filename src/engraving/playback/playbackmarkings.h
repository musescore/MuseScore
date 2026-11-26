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

#include <vector>

#include "engraving/types/symid.h"

//! Which markings the note playback Info panel lists, and the strings it shows for them. The canvas uses the same
//! tables to highlight everything the panel would list, so the two always agree.
namespace mu::engraving {
class EngravingItem;

//! One line of a marking's row in the Info panel: its glyph, then its name; either may be missing
struct MarkingLine {
    SymId glyph = SymId::noSym;
    const char* name = nullptr; // untranslated, in the "propertiespanel" context
};

//! What a marking shows in each section of the Info panel (Properties > Playback). A section's first line is the marking's row, the others are
//! listed under it. A marking with no lines in any section isn't shown.
struct MarkingDescription {
    std::vector<MarkingLine> tempo {};
    std::vector<MarkingLine> dynamic {};
    std::vector<MarkingLine> other {};
};

//! How the strings table describes the item: nullptr for kinds of items it doesn't cover (they're listed by what they
//! do), a description with no lines for those it hides (notehead changes, and kinds left out for now)
const MarkingDescription* markingDescription(const EngravingItem* item);

//! Whether the Info panel lists the item as a marking: the table either doesn't cover it or gives it a line
bool isListedMarking(const EngravingItem* item);
} // namespace mu::engraving
