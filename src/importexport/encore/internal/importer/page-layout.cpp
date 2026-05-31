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

// Page and printer geometry for the Encore importer (see page-layout.h).

#include "page-layout.h"

#include <algorithm>
#include <cmath>
#include <set>

#include <QPageSize>

#include "ctx.h"
#include "../parser/elem.h"
#include "../parser/readers.h"

#include "engraving/dom/factory.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/page.h"
#include "engraving/dom/system.h"
#include "engraving/dom/systemlock.h"
#include "engraving/style/style.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::iex::enc {

// Map a Windows DEVMODE dmPaperSize (DMPAPER_*) to a Qt page size. Returns Custom for
// values without a standard mapping; the caller then falls back to dmPaperLength/Width or
// to the WINI geometry heuristic.
static QPageSize::PageSizeId dmPaperToQt(int dmPaper)
{
    switch (dmPaper) {
    case 1:  return QPageSize::Letter;
    case 5:  return QPageSize::Legal;
    case 7:  return QPageSize::Executive;
    case 8:  return QPageSize::A3;
    case 9:  return QPageSize::A4;
    case 11: return QPageSize::A5;
    case 12: return QPageSize::B4;     // DMPAPER_B4 (JIS)
    case 13: return QPageSize::B5;     // DMPAPER_B5 (JIS)
    default: return QPageSize::Custom;
    }
}

// Display size (1-4) for an instrument: per-instrument staffSizeHint from the LINE staff entry,
// falling back to the global header.scoreSize for files without LINE data.
// See ENCORE_FORMAT.md §System block (LINE).
int staffDisplaySize(const EncRoot& enc, int instrIdx)
{
    if (!enc.lines.empty()) {
        for (const EncLineStaffData& lsd : enc.lines[0].staffData) {
            if (static_cast<int>(lsd.instrumentIndex()) == instrIdx) {
                return std::clamp(static_cast<int>(lsd.staffSizeHint) + 1, 1, 4);
            }
        }
    }
    return std::clamp(static_cast<int>(enc.header.scoreSize), 1, 4);
}

// Inclusive [firstBlock, lastBlock] MEAS-block range covered by line[li]. Prefer the stored
// per-line measureCount; when it is absent (0, as in SCO5) fall back to the gap to the next
// line's start, or to totalBlocks for the last line. lastBlock < firstBlock when it spans nothing.
struct LineBlockSpan {
    int firstBlock;
    int lastBlock;
};
} // namespace mu::iex::enc
