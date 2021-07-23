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
#include "writescorehook.h"

#include "libmscore/masterscore.h"
#include "libmscore/xml.h"

#include "config.h"

using namespace mu::engraving::compat;

void WriteScoreHook::onWriteStyle302(Ms::Score* score, Ms::XmlWriter& xml)
{
    bool isWriteStyle = false;
    //! NOTE Write the style to the score file if the compatibility define is set
#ifdef ENGRAVING_COMPAT_WRITESTYLE_302
    isWriteStyle = true;
#endif

    //! NOTE If not the master score, because the Excerpts (parts) have not yet been write to separate files
    if (!score->isMaster()) {
        isWriteStyle = true;
    }

    //! NOTE If the test mode, because the tests have not yet been adapted to the new format
    if (Ms::MScore::testMode) {
        isWriteStyle = true;
    }

    if (isWriteStyle) {
        score->style().save(xml, true);     // save only differences to buildin style (logic from 3.)
    }
}
