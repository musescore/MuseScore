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
#include "writescorehook.h"

#include "rw/xmlwriter.h"

#include "dom/masterscore.h"
#include "dom/excerpt.h"

#include "rw/write/writer.h"

using namespace mu::engraving;
using namespace mu::engraving::compat;
using namespace mu::engraving::write;

void WriteScoreHook::onWriteStyle302(Score* score, XmlWriter& xml)
{
    bool isWriteStyle = false;

    //! NOTE If not the master score, because the Excerpts (parts) have not yet been write to separate files
    if (!score->isMaster()) {
        isWriteStyle = true;
    }

    //! NOTE If the test mode, because the tests have not yet been adapted to the new format
    if (MScore::testMode && MScore::testWriteStyleToScore) {
        isWriteStyle = true;
    }

    if (isWriteStyle) {
        score->style().save(xml, true);     // save only differences to builtin style (logic from 3.)
    }
}

void WriteScoreHook::onWriteExcerpts302(Score* score, XmlWriter& xml, WriteContext& ctx, bool selectionOnly)
{
    bool isWriteExcerpts = false;

    //! NOTE If the test mode, because the tests have not yet been adapted to the new format
    if (MScore::testMode) {
        isWriteExcerpts = true;
    }

    if (isWriteExcerpts && score->isMaster() && !selectionOnly) {
        MasterScore* mScore = static_cast<MasterScore*>(score);
        for (const Excerpt* excerpt : mScore->excerpts()) {
            if (excerpt->excerptScore() != score) {
                write::Writer::write(excerpt->excerptScore(), xml, ctx, selectionOnly, *this);         // recursion write
            }
        }
    }
}
