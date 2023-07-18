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

#include "registeraudiopluginsscenario.h"

#include <QApplication>

#include "audioutils.h"
#include "audioerrors.h"
#include "translation.h"
#include "log.h"

using namespace mu::audio;
using namespace mu::framework;

void RegisterAudioPluginsScenario::init()
{
    TRACEFUNC;

    m_progress.cancelRequested.onNotify(this, [this]() {
        m_aborted = true;
    });

    Ret ret = knownPluginsRegister()->load();
    if (!ret) {
        LOGE() << ret.toString();
    }
}

mu::Ret RegisterAudioPluginsScenario::registerNewPlugins()
{
    TRACEFUNC;

    io::paths_t newPluginPaths;

    for (IAudioPluginsScannerPtr scanner : scannerRegister()->scanners()) {
        io::paths_t paths = scanner->scanPlugins();

        for (const io::path_t& path : paths) {
            if (!knownPluginsRegister()->exists(path)) {
                newPluginPaths.push_back(path);
            }
        }
    }

    if (newPluginPaths.empty()) {
        return make_ok();
    }

    processPluginsRegistration(newPluginPaths);

    Ret ret = knownPluginsRegister()->load();
    return ret;
}

void RegisterAudioPluginsScenario::processPluginsRegistration(const io::paths_t& pluginPaths)
{
    Ret ret = interactive()->showProgress(mu::trc("audio", "Scanning audio plugins"), &m_progress);
    if (!ret) {
        LOGE() << ret.toString();
    }

    m_aborted = false;
    m_progress.started.notify();

    std::string appPath = globalConfiguration()->appBinPath().toStdString();
    int64_t pluginCount = static_cast<int64_t>(pluginPaths.size());

    for (int64_t i = 0; i < pluginCount; ++i) {
        if (m_aborted) {
            return;
        }

        const io::path_t& pluginPath = pluginPaths[i];
        std::string pluginPathStr = pluginPath.toStdString();

        m_progress.progressChanged.send(i, pluginCount, io::filename(pluginPath).toStdString());
        qApp->processEvents();

        int code = process()->execute(appPath, { "--register-audio-plugin", pluginPathStr });
        if (code != 0) {
            code = process()->execute(appPath, { "--register-failed-audio-plugin", pluginPathStr, "--", std::to_string(code) });
        }

        if (code != 0) {
            LOGE() << "Could not register plugin: " << pluginPathStr << "\n error code: " << code;
        }
    }

    m_progress.finished.send(make_ok());
}

mu::Ret RegisterAudioPluginsScenario::registerPlugin(const io::path_t& pluginPath)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(!pluginPath.empty()) {
        return false;
    }

    IAudioPluginMetaReaderPtr reader = metaReader(pluginPath);
    if (!reader) {
        return make_ret(audio::Err::UnknownPluginType);
    }

    RetVal<AudioResourceMetaList> metaList = reader->readMeta(pluginPath);
    if (!metaList.ret) {
        LOGE() << metaList.ret.toString();
        return metaList.ret;
    }

    for (const AudioResourceMeta& meta : metaList.val) {
        AudioPluginInfo info;
        info.type = audioPluginTypeFromCategoriesString(meta.attributeVal(audio::CATEGORIES_ATTRIBUTE));
        info.meta = meta;
        info.path = pluginPath;
        info.enabled = true;

        Ret ret = knownPluginsRegister()->registerPlugin(info);
        if (!ret) {
            return ret;
        }
    }

    return make_ok();
}

mu::Ret RegisterAudioPluginsScenario::registerFailedPlugin(const io::path_t& pluginPath, int failCode)
{
    TRACEFUNC;

    IF_ASSERT_FAILED(!pluginPath.empty()) {
        return false;
    }

    AudioPluginInfo info;
    info.meta.id = io::completeBasename(pluginPath).toStdString();

    std::string ext = io::suffix(pluginPath);
    if (ext.find("vst") != std::string::npos) {
        info.meta.type = AudioResourceType::VstPlugin;
    }

    info.path = pluginPath;
    info.enabled = false;
    info.errorCode = failCode;

    Ret ret = knownPluginsRegister()->registerPlugin(info);
    return ret;
}

IAudioPluginMetaReaderPtr RegisterAudioPluginsScenario::metaReader(const io::path_t& pluginPath) const
{
    for (IAudioPluginMetaReaderPtr reader : metaReaderRegister()->readers()) {
        if (reader->canReadMeta(pluginPath)) {
            return reader;
        }
    }

    return nullptr;
}
