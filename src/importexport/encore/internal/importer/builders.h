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

// Top-level build pipeline entry points: create parts, create measures, set initial signatures,
// then emit the note/rest/element content into the score.

#ifndef MU_IMPORTEXPORT_ENC_IMPORT_BUILDERS_H
#define MU_IMPORTEXPORT_ENC_IMPORT_BUILDERS_H

#include "ctx.h"

namespace mu::iex::enc {
void buildParts(BuildCtx& ctx);
void buildMeasures(BuildCtx& ctx);
void buildInitialSignatures(BuildCtx& ctx);
void emitMeasures(BuildCtx& ctx);
// Post-pass applying the tablature import mode: Separate keeps buildParts' independent tab staves,
// Linked merges each tab staff into its adjacent matching notation staff as a linked clone, and
// Ignore drops tab staves. Runs after notes are emitted and resolved, before final layout.
void applyTablatureImportMode(BuildCtx& ctx);
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_IMPORT_BUILDERS_H
