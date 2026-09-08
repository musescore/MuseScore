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
#include "ireader.h"

#include "dom/excerpt.h"
#include "dom/masterscore.h"

using namespace mu::engraving;
using namespace mu::engraving::rw;

/// Default implementation: assumes all excerpts are initialised (pre-500 formats).
/// Creates a Score, applies style, and delegates to readScoreFile.
muse::Ret IReader::readExcerptScoreFile(Excerpt* excerpt, MasterScore* masterScore,
                                        const muse::ByteArray& excerptData,
                                        const muse::String& fileName,
                                        ReadInOutData* out,
                                        const ApplyStyleFn& applyStyleFn)
{
    Score* partScore = masterScore->createScore();
    excerpt->setExcerptScore(partScore);

    if (applyStyleFn) {
        applyStyleFn(partScore);
    }

    XmlReader xml(excerptData);
    xml.setDocName(fileName);

    muse::Ret ret = readScoreFile(partScore, xml, out);
    if (!ret) {
        return ret;
    }

    Excerpt::linkMeasures(partScore, masterScore);

    return muse::make_ok();
}
