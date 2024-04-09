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

#include "diagnosticfileswriter.h"

#include "serialization/zipwriter.h"
#include "containers.h"

#include "log.h"

using namespace muse::diagnostics;
using namespace muse;
using namespace muse::io;

Ret DiagnosticFilesWriter::writeDiagnosticFiles(const path_t& destinationPath)
{
    TRACEFUNC;

    const std::vector<std::string> DIRS_TO_WRITE {
        "logs",
        "plugins",
        "workspaces",
    };

    ZipWriter zip(destinationPath);

    for (const std::string& dirName : DIRS_TO_WRITE) {
        RetVal<io::paths_t> files = scanDir(dirName);
        if (!files.ret) {
            LOGE() << files.ret;
            continue;
        }

        for (const muse::io::path_t& filePath : files.val) {
            Ret ret = addFileToZip(filePath, zip, dirName);
            if (!ret) {
                LOGE() << ret.toString();
            }
        }
    }

    const std::vector<std::string> FILES_TO_WRITE {
        "shortcuts.xml",
        "midi_mappings.xml",
        "known_audio_plugins.json",
    };

    for (const std::string& fileName : FILES_TO_WRITE) {
        Ret ret = addFileToZip(globalConfiguration()->userAppDataPath() + "/" + fileName, zip);
        if (!ret) {
            LOGE() << ret.toString();
        }
    }

    return muse::make_ok();
}

RetVal<muse::io::paths_t> DiagnosticFilesWriter::scanDir(const std::string& dirName)
{
    RetVal<io::paths_t> paths = fileSystem()->scanFiles(globalConfiguration()->userAppDataPath() + "/" + dirName,
                                                        { "*" },
                                                        ScanMode::FilesInCurrentDirAndSubdirs);
    return paths;
}

Ret DiagnosticFilesWriter::addFileToZip(const path_t& filePath, ZipWriter& zip, const std::string& destinationDirName)
{
    RetVal<ByteArray> data = fileSystem()->readFile(filePath);
    if (!data.ret) {
        return data.ret;
    }

    std::string fileName = io::filename(filePath).toStdString();
    std::string filePathInZip = destinationDirName.empty() ? fileName : destinationDirName + "/" + fileName;

    zip.addFile(filePathInZip, data.val);

    return muse::make_ok();
}
