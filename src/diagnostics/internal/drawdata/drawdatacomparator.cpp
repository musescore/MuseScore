/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#include "../../diagnosticserrors.h"

using namespace mu;
using namespace muse::draw;
using namespace mu::diagnostics;

Diff DrawDataComparator::compare(const DrawDataPtr& ref, const DrawDataPtr& test)
{
    Diff diff = DrawDataComp::compare(ref, test);
    return diff;
}

Ret DrawDataComparator::compare(const io::path_t& ref, const io::path_t& test, const io::path_t& outdiff)
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
        return make_ok();
    }

    io::FileInfo(outdiff).dir().mkpath();

    DrawDataRW::writeDiff(outdiff, diff);
    return make_ret(Err::DDiff);
}
