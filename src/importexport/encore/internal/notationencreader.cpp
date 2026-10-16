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

// Collects import options from the configuration, runs importEncore, and maps errors to messages.

#include "notationencreader.h"

#include "importer/import.h"

#include "engraving/engravingerrors.h"

using namespace mu::iex::enc;
using namespace mu::engraving;

muse::Ret NotationEncoreReader::read(MasterScore* score, const muse::io::path_t& path, const Options&)
{
    EncImportOptions opts;
    opts.importPageLayout                     = encoreConfiguration()->importPageLayout();
    opts.importPageBreaks                     = encoreConfiguration()->importPageBreaks();
    opts.importSystemLocks                    = encoreConfiguration()->importSystemLocks();
    opts.importStaffSize                      = encoreConfiguration()->importStaffSize();
    opts.importTempoTextSemantic              = encoreConfiguration()->importTempoTextSemantic();
    opts.importUnsupportedArticulationsAsText = encoreConfiguration()->importUnsupportedArticulationsAsText();
    opts.instrumentSearchMode                  = encoreConfiguration()->instrumentSearchMode();
    opts.underfillMeasureStrategy             = encoreConfiguration()->underfillMeasureStrategy();
    opts.overfillMeasureStrategy              = encoreConfiguration()->overfillMeasureStrategy();
    opts.firstMeasureIsPickup                 = encoreConfiguration()->firstMeasureIsPickup();
    opts.mergeVoices                          = encoreConfiguration()->mergeVoices();
    opts.tablatureImportMode                  = encoreConfiguration()->tablatureImportMode();
    Err err = importEncore(score, path.toQString(), opts);
    if (err == Err::FileBadFormat) {
        // Give a specific reason instead of the generic "Bad format" text.
        return make_ret(err, encoreLoadErrorMessage(path.toQString()));
    }
    return make_ret(err, path);
}
