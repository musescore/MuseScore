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
#include "diagnosticdrawprovider.h"

#include "global/io/fileinfo.h"
#include "global/io/file.h"

#include "drawdatagenerator.h"
#include "drawdataconverter.h"
#include "drawdatacomparator.h"

#include "log.h"

using namespace muse;
using namespace mu::engraving;

// --diagnostic-gen-drawdata ./vtest/scores --diagnostic-output ./drawdata
// --diagnostic-gen-drawdata ./vtest/scores/accidental-1.mscx --diagnostic-output ./drawdata/accidental-1.json
// --diagnostic-com-drawdata ./drawdata/accidental-1.json ./drawdata/accidental-2.json --diagnostic-output ./drawdata/accidental-1-2.diff.json
// --diagnostic-drawdata-to-png ./drawdata/accidental-1.json --diagnostic-output ./drawdata/accidental-1.png
// --diagnostic-drawdiff-to-png ./drawdata/accidental-1-2.diff.json ./drawdata/accidental-1.json --diagnostic-output ./drawdata/accidental-1-2.diff.png
// ./vtest/scores/accidental-1.mscx -o ./work/1_accidental-1.exp.png
// ./vtest/scores/emmentaler-text-3.mscx -o ./work/emmentaler-text-3.png

Ret DiagnosticDrawProvider::generateDrawData(const muse::io::path_t& dirOrFile, const muse::io::path_t& outDirOrFile, const GenOpt& opt)
{
    LOGI() << "scoresDir: " << dirOrFile << ", outDir: " << outDirOrFile;
    DrawDataGenerator g(iocContext());

    if (io::FileInfo(dirOrFile).entryType() == io::EntryType::File) {
        return g.processFile(dirOrFile, outDirOrFile, opt);
    }

    return g.processDir(dirOrFile, outDirOrFile, opt);
}

Ret DiagnosticDrawProvider::compareDrawData(const muse::io::path_t& ref, const muse::io::path_t& test, const muse::io::path_t& outDiff,
                                            const ComOpt& opt)
{
    LOGI() << "ref: " << ref << ", test: " << test << ", outDiff: " << outDiff;
    DrawDataComparator c;
    Ret ret = c.compare(ref, test, outDiff);

    // no diff
    if (ret) {
        return ret;
    }

    muse::io::path_t outDir = io::FileInfo(outDiff).dirPath();
    if (opt.isCopySrc) {
        io::File::copy(ref, outDir + "/" + io::FileInfo(ref).completeBaseName() + ".ref.json");
        io::File::copy(test, outDir + "/" + io::FileInfo(test).completeBaseName() + ".json");
    }

    if (opt.isMakePng) {
        DrawDataConverter c2;
        c2.drawDataToPng(ref, outDir + "/" + io::FileInfo(ref).completeBaseName() + ".ref.png");
        c2.drawDataToPng(test, outDir + "/" + io::FileInfo(test).completeBaseName() + ".png");
        c2.drawDiffToPng(outDiff, ref, outDir + "/" + io::FileInfo(outDiff).completeBaseName() + ".diff.png");
    }

    return ret;
}

Ret DiagnosticDrawProvider::drawDataToPng(const muse::io::path_t& dataFile, const muse::io::path_t& outFile)
{
    LOGI() << "dataFile: " << dataFile << ", outFile: " << outFile;
    DrawDataConverter c;
    return c.drawDataToPng(dataFile, outFile);
}

Ret DiagnosticDrawProvider::drawDiffToPng(const muse::io::path_t& diffFile, const muse::io::path_t& refFile,
                                          const muse::io::path_t& outFile)
{
    LOGI() << "diffFile: " << diffFile << ", refFile: " << refFile << ", outFile: " << outFile;
    DrawDataConverter c;
    return c.drawDiffToPng(diffFile, refFile, outFile);
}
