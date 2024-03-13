/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "extensionsloader.h"

#include "global/io/dir.h"
#include "global/io/file.h"
#include "global/io/fileinfo.h"
#include "global/serialization/json.h"

#include "log.h"

const std::string MANIFEST("manifest.json");

using namespace mu::extensions;

ManifestList ExtensionsLoader::loadManifesList(const io::path_t& defPath, const io::path_t& extPath) const
{
    TRACEFUNC;

    LOGD() << "try load extensions, def: " << defPath << ", user: " << extPath;

    ManifestList defaultManifests = manifesList(defPath);
    ManifestList externalManifests = manifesList(extPath);

    ManifestList retList;
    for (const Manifest& m : defaultManifests) {
        if (!m.enabled || !m.isValid()) {
            continue;
        }
        retList.push_back(m);
    }

    for (const Manifest& m : externalManifests) {
        if (!m.enabled || !m.isValid()) {
            continue;
        }
        retList.push_back(m);
    }

    return retList;
}

ManifestList ExtensionsLoader::manifesList(const io::path_t& rootPath) const
{
    ManifestList manifests;
    io::paths_t paths = manifestPaths(rootPath);
    for (const io::path_t& path : paths) {
        Manifest manifest = parseManifest(path);
        resolvePaths(manifest, io::FileInfo(path).dirPath());
        manifests.push_back(manifest);
    }

    return manifests;
}

mu::io::paths_t ExtensionsLoader::manifestPaths(const io::path_t& rootPath) const
{
    RetVal<io::paths_t> paths = io::Dir::scanFiles(rootPath, { MANIFEST });
    if (!paths.ret) {
        LOGE() << "failed scan files, err: " << paths.ret.toString();
    }
    return paths.val;
}

Manifest ExtensionsLoader::parseManifest(const io::path_t& path) const
{
    ByteArray data;
    Ret ret = io::File::readFile(path, data);
    if (!ret) {
        LOGE() << "failed read file: " << path << ", err: " << ret.toString();
        return Manifest();
    }

    std::string jsonErr;
    JsonObject obj = JsonDocument::fromJson(data, &jsonErr).rootObject();
    if (!jsonErr.empty()) {
        LOGE() << "failed parse json, err: " << jsonErr;
        return Manifest();
    }

    Manifest m;
    m.uri = Uri(obj.value("uri").toStdString());
    m.type = typeFromString(obj.value("type").toStdString());
    m.title = obj.value("title").toString();
    m.apiversion = obj.value("apiversion", 1).toInt();
    m.enabled = obj.value("enabled", true).toBool();
    m.visible = obj.value("enabled", true).toBool();
    m.qmlFilePath = obj.value("qmlFilePath").toStdString();

    return m;
}

void ExtensionsLoader::resolvePaths(Manifest& m, const io::path_t& rootDirPath) const
{
    m.qmlFilePath = rootDirPath + "/" + m.qmlFilePath;
}
