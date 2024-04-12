/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#include "drawdatacomparator.h"

#include "global/io/fileinfo.h"
#include "global/io/dir.h"

#include "draw/utils/drawdatacomp.h"
#include "draw/utils/drawdatarw.h"

#include "drawdataerrors.h"

using namespace muse;
using namespace muse::draw;
using namespace mu::engraving;

Diff DrawDataComparator::compare(const DrawDataPtr& ref, const DrawDataPtr& test)
{
    Diff diff = DrawDataComp::compare(ref, test);
    return diff;
}

Ret DrawDataComparator::compare(const muse::io::path_t& ref, const muse::io::path_t& test, const muse::io::path_t& outdiff)
{
    RetVal<DrawDataPtr> refData = DrawDataRW::readData(ref);
    if (!refData.ret) {
        return refData.ret;
    }

    RetVal<DrawDataPtr> testData = DrawDataRW::readData(test);
    if (!testData.ret) {
        return testData.ret;
    }

    Diff diff = DrawDataComp::compare(refData.val, testData.val);

    if (diff.empty()) {
        return muse::make_ok();
    }

    io::FileInfo(outdiff).dir().mkpath();

    DrawDataRW::writeDiff(outdiff, diff);
    return make_ret(Err::DDiff);
}
