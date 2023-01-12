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
#include "engravingdrawprovider.h"

#include "drawdatagenerator.h"
#include "drawdataconverter.h"

#include "log.h"

using namespace mu;
using namespace mu::diagnostics;

// --diagnostic-gen-drawdata ./vtest/scores --diagnostic-output ./drawdata
// --diagnostic-drawdata-to-png ./drawdata/accidental-24.json --diagnostic-output ./drawdata/accidental-24.png

Ret EngravingDrawProvider::genDrawData(const io::path_t& scoresDir, const io::path_t& outDir)
{
    LOGI() << "scoresDir: " << scoresDir << ", outDir: " << outDir;
    DrawDataGenerator g;
    return g.processDir(scoresDir, outDir, io::path_t());
}

Ret EngravingDrawProvider::drawDataToPng(const io::path_t& dataFile, const io::path_t& outFile)
{
    DrawDataConverter c;
    return c.drawDataToPng(dataFile, outFile);
}
