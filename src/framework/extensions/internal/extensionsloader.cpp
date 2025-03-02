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
#include "global/stringutils.h"
#include "global/runtime.h"

#include "log.h"

using namespace muse;
using namespace muse::extensions;

const std::string MANIFEST("manifest.json");
const std::string DEV_EXTENSIONS("extensions/dev/");

ManifestList ExtensionsLoader::loadManifestList(const io::path_t& defPath, const io::path_t& extPath) const
{
    TRACEFUNC;

    LOGD() << "try load extensions, def: " << defPath << ", user: " << extPath;

    ManifestList defaultManifests = manifestList(defPath);
    ManifestList externalManifests = manifestList(extPath);

    ManifestList retList;
    for (const Manifest& m : defaultManifests) {
        if (!m.isValid()) {
            continue;
        }

        if (!runtime::isDebug() && muse::strings::startsWith(m.uri.path(), DEV_EXTENSIONS)) {
            continue;
        }

        retList.push_back(m);
    }

    for (const Manifest& m : externalManifests) {
        if (!m.isValid()) {
            continue;
        }

        if (!runtime::isDebug() && muse::strings::startsWith(m.uri.path(), DEV_EXTENSIONS)) {
            continue;
        }

        retList.push_back(m);
    }

    return retList;
}

ManifestList ExtensionsLoader::manifestList(const io::path_t& rootPath) const
{
    ManifestList manifests;
    io::paths_t paths = manifestPaths(rootPath);
    for (const io::path_t& path : paths) {
        LOGD() << "parsing manifest: " << path;
        Manifest manifest = parseManifest(path);
        resolvePaths(manifest, io::FileInfo(path).dirPath());
        manifests.push_back(manifest);
    }

    return manifests;
}

io::paths_t ExtensionsLoader::manifestPaths(const io::path_t& rootPath) const
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

    return parseManifest(data);
}

Manifest ExtensionsLoader::parseManifest(const ByteArray& data) const
{
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
    m.description = obj.value("description").toString();
    m.category = obj.value("category").toString();
    m.thumbnail = obj.value("thumbnail").toStdString();
    m.version = obj.value("version").toString();
    m.apiversion = obj.value("apiversion", DEFAULT_API_VERSION).toInt();

    String uiCtx = obj.value("ui_context", String(DEFAULT_UI_CONTEXT)).toString();
    if (obj.contains("actions")) {
        JsonArray arr = obj.value("actions").toArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            JsonObject ao = arr.at(i).toObject();
            Action a;
            a.code = ao.value("code").toStdString();
            a.type = ao.contains("type") ? typeFromString(ao.value("type").toStdString()) : m.type;
            a.modal = ao.value("modal", DEFAULT_MODAL).toBool();
            a.title = ao.value("title").toString();
            std::string icon = ao.value("icon").toStdString();
            a.icon = ui::IconCode::fromString(icon.c_str());
            a.uiCtx = ao.value("ui_context", uiCtx).toString();
            a.showOnAppmenu = ao.value("show_on_appmenu", true).toBool();
            a.showOnToolbar = ao.value("show_on_toolbar", false).toBool();
            a.path = ao.value("path").toStdString();
            a.func = ao.value("func", "main").toString();
            a.apiversion = m.apiversion;
            m.actions.push_back(std::move(a));
        }
    } else {
        Action a;
        a.code = "main";
        a.type = m.type;
        a.modal = obj.value("modal", DEFAULT_MODAL).toBool();
        a.title = m.title;
        a.uiCtx = uiCtx;
        a.path = obj.value("path").toStdString();
        a.func = u"main";
        a.apiversion = m.apiversion;
        m.actions.push_back(std::move(a));
    }

    return m;
}

void ExtensionsLoader::resolvePaths(Manifest& m, const io::path_t& rootDirPath) const
{
    if (!m.thumbnail.empty()) {
        m.thumbnail = rootDirPath + "/" + m.thumbnail;
    }

    for (Action& a : m.actions) {
        a.path = rootDirPath + "/" + a.path;
    }
}
