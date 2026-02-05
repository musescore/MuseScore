/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "registeraudiopluginsscenario.h"

#include <QCoreApplication>
#include <map>

#include "global/translation.h"

#include "audiopluginserrors.h"
#include "audiopluginsutils.h"

#include "log.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audioplugins;

void RegisterAudioPluginsScenario::init()
{
    TRACEFUNC;

    m_progress.canceled().onNotify(this, [this]() {
        m_aborted = true;
    });

    Ret ret = knownPluginsRegister()->load();
    if (!ret) {
        LOGE() << ret.toString();
    }
}

PluginScanResult RegisterAudioPluginsScenario::scanPlugins() const
{
    TRACEFUNC;

    PluginScanResult result;

    std::map<io::path_t, audio::AudioResourceId> registered;
    for (const auto& info : knownPluginsRegister()->pluginInfoList()) {
        registered[info.path] = info.meta.id;
    }

    for (const auto& scanner : scannerRegister()->scanners()) {
        for (const auto& path : scanner->scanPlugins()) {
            if (auto it = registered.find(path); it != registered.end()) {
                registered.erase(it);
            } else {
                result.newPluginPaths.push_back(path);
            }
        }
    }

    for (const auto& [path, id] : registered) {
        result.missingPluginIds.push_back(id);
    }

    return result;
}

Ret RegisterAudioPluginsScenario::updatePluginsRegistry()
{
    TRACEFUNC;

    PluginScanResult result = scanPlugins();

    unregisterRemovedPlugins(result.missingPluginIds);
    registerNewPlugins(result.newPluginPaths);

    return knownPluginsRegister()->load();
}

void RegisterAudioPluginsScenario::registerNewPlugins(const io::paths_t& pluginPaths)
{
    TRACEFUNC;

    if (pluginPaths.empty()) {
        return;
    }

    processPluginsRegistration(pluginPaths);
    knownPluginsRegister()->load();
}

Ret RegisterAudioPluginsScenario::unregisterRemovedPlugins(const audio::AudioResourceIdList& pluginIds)
{
    TRACEFUNC;

    if (pluginIds.empty()) {
        return make_ok();
    }

    Ret ret = knownPluginsRegister()->unregisterPlugins(pluginIds);
    if (!ret) {
        LOGE() << "Failed to unregister removed plugins: " << ret.toString();
    }

    return ret;
}

void RegisterAudioPluginsScenario::processPluginsRegistration(const io::paths_t& pluginPaths)
{
    interactive()->showProgress(muse::trc("audio", "Scanning audio plugins"), m_progress);

    m_aborted = false;
    m_progress.start();

    std::string appPath = globalConfiguration()->appBinPath().toStdString();
    int64_t pluginCount = static_cast<int64_t>(pluginPaths.size());

    for (int64_t i = 0; i < pluginCount; ++i) {
        if (m_aborted) {
            return;
        }

        const io::path_t& pluginPath = pluginPaths[i];
        std::string pluginPathStr = pluginPath.toStdString();

        m_progress.progress(i, pluginCount, io::filename(pluginPath).toStdString());
        qApp->processEvents();

        LOGD() << "--register-audio-plugin " << pluginPathStr;
        int code = process()->execute(appPath, { "--register-audio-plugin", pluginPathStr });
        if (code != 0) {
            code = process()->execute(appPath, { "--register-failed-audio-plugin", pluginPathStr, "--", std::to_string(code) });
        }

        if (code != 0) {
            LOGE() << "Could not register plugin: " << pluginPathStr << "\n error code: " << code;
        }
    }

    m_progress.finish(muse::make_ok());
}

Ret RegisterAudioPluginsScenario::registerPlugin(const io::path_t& pluginPath)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(!pluginPath.empty()) {
        return false;
    }

    const IAudioPluginMetaReaderPtr reader = metaReader(pluginPath);
    if (!reader) {
        return make_ret(Err::UnknownPluginType);
    }

    const RetVal<AudioResourceMetaList> metaList = reader->readMeta(pluginPath);
    if (!metaList.ret) {
        LOGE() << metaList.ret.toString();
        return metaList.ret;
    }

    AudioPluginInfoList infoList;
    infoList.reserve(metaList.val.size());

    for (const AudioResourceMeta& meta : metaList.val) {
        AudioPluginInfo info;
        info.type = audioPluginTypeFromCategoriesString(meta.attributeVal(audio::CATEGORIES_ATTRIBUTE));
        info.meta = meta;
        info.path = pluginPath;
        info.enabled = true;
        infoList.emplace_back(std::move(info));
    }

    Ret ret = knownPluginsRegister()->registerPlugins(infoList);
    return ret;
}

Ret RegisterAudioPluginsScenario::registerFailedPlugin(const io::path_t& pluginPath, int failCode)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(!pluginPath.empty()) {
        return false;
    }

    AudioPluginInfo info;
    info.meta.id = io::completeBasename(pluginPath).toStdString();
    info.meta.type = metaType(pluginPath);
    info.path = pluginPath;
    info.enabled = false;
    info.errorCode = failCode;

    Ret ret = knownPluginsRegister()->registerPlugins({ info });
    return ret;
}

IAudioPluginMetaReaderPtr RegisterAudioPluginsScenario::metaReader(const io::path_t& pluginPath) const
{
    for (const IAudioPluginMetaReaderPtr& reader : metaReaderRegister()->readers()) {
        if (reader->canReadMeta(pluginPath)) {
            return reader;
        }
    }

    return nullptr;
}

audio::AudioResourceType RegisterAudioPluginsScenario::metaType(const io::path_t& pluginPath) const
{
    const IAudioPluginMetaReaderPtr reader = metaReader(pluginPath);
    return reader ? reader->metaType() : audio::AudioResourceType::Undefined;
}
