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

// Public entry point of the Encore (.enc) importer: parse a file into a MasterScore, plus the
// helper that turns a rejected file into a user-facing error message.
//
// The binary format was reverse-engineered by Leon Vinken (Enc2MusicXML project,
// https://github.com/lvinken/Enc2MusicXML, GPL v3+) building on enc2ly by Felipe Castro.
// This importer is based on that work.

#ifndef MU_IMPORTEXPORT_ENC_IMPORT_IMPORT_H
#define MU_IMPORTEXPORT_ENC_IMPORT_IMPORT_H

#include "engraving/engravingerrors.h"
#include "import-options.h"

namespace mu::engraving {
class MasterScore;
}

namespace mu::iex::enc {
mu::engraving::Err importEncore(mu::engraving::MasterScore* score, const QString& path, const EncImportOptions& opts = EncImportOptions {});

// Build a user-facing message for a file importEncore rejected with FileBadFormat, by
// re-reading its header: an older encrypted Encore container (ZBOT/ZBOP/ZBO6), a SCOW/SCO5
// file that could not be parsed (unsupported variant, damaged, or empty), or a file with no
// recognizable Encore header at all.
muse::String encoreLoadErrorMessage(const QString& path);
} // namespace mu::iex::enc

#endif // MU_IMPORTEXPORT_ENC_IMPORT_IMPORT_H
