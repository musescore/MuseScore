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

#include "global/io/fileinfo.h"

#include "drawdatagenerator.h"
#include "drawdataconverter.h"
#include "drawdatacomparator.h"

#include "log.h"

using namespace mu;
using namespace mu::diagnostics;

// --diagnostic-gen-drawdata ./vtest/scores --diagnostic-output ./drawdata
// --diagnostic-gen-drawdata ./vtest/scores/accidental-1.mscx --diagnostic-output ./drawdata/accidental-1.json
// --diagnostic-com-drawdata ./drawdata/accidental-1.json ./drawdata/accidental-2.json --diagnostic-output ./drawdata/accidental-1-2.diff.json
// --diagnostic-drawdata-to-png ./drawdata/accidental-1.json --diagnostic-output ./drawdata/accidental-1.png
// --diagnostic-drawdiff-to-png ./drawdata/accidental-1-2.diff.json ./drawdata/accidental-1.json --diagnostic-output ./drawdata/accidental-1-2.diff.png
//  ./vtest/scores/accidental-1.mscx -o ./drawdata/1_accidental-1.exp.png

Ret EngravingDrawProvider::generateDrawData(const io::path_t& dirOrFile, const io::path_t& outDirOrFile)
{
    LOGI() << "scoresDir: " << dirOrFile << ", outDir: " << outDirOrFile;
    DrawDataGenerator g;

    if (io::FileInfo(dirOrFile).entryType() == io::EntryType::File) {
        return g.processFile(dirOrFile, outDirOrFile);
    }

    return g.processDir(dirOrFile, outDirOrFile, io::path_t());
}

Ret EngravingDrawProvider::compareDrawData(const io::path_t& ref, const io::path_t& test, const io::path_t& outDiff)
{
    LOGI() << "ref: " << ref << ", test: " << test << ", outDiff: " << outDiff;
    DrawDataComparator c;
    return c.compare(ref, test, outDiff);
}

Ret EngravingDrawProvider::drawDataToPng(const io::path_t& dataFile, const io::path_t& outFile)
{
    LOGI() << "dataFile: " << dataFile << ", outFile: " << outFile;
    DrawDataConverter c;
    return c.drawDataToPng(dataFile, outFile);
}

Ret EngravingDrawProvider::drawDiffToPng(const io::path_t& diffFile, const io::path_t& refFile, const io::path_t& outFile)
{
    LOGI() << "diffFile: " << diffFile << ", refFile: " << refFile << ", outFile: " << outFile;
    DrawDataConverter c;
    return c.drawDiffToPng(diffFile, refFile, outFile);
}
