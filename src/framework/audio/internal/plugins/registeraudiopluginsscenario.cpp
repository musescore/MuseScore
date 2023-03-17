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

#include "translation.h"
#include "log.h"

using namespace mu::audio;
using namespace mu::framework;

void RegisterAudioPluginsScenario::init()
{
    m_progress.finished.onReceive(this, [this](const ProgressResult& res) {
        if (res.ret.code() == static_cast<int>(Ret::Code::Cancel)) {
            m_aborted = true;
        }
    });
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

    startPluginsRegistration(newPluginPaths);

    return make_ok();
}

void RegisterAudioPluginsScenario::startPluginsRegistration(const io::paths_t& pluginPaths)
{
    if (pluginPaths.empty()) {
        return;
    }

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

        m_progress.progressChanged.send(i, pluginCount, pluginPathStr);
        qApp->processEvents();

        int code = process()->execute(appPath, { "--register-audio-plugin", pluginPathStr });
        if (code == 0) {
            continue;
        }

        AudioPluginInfo info;
        info.meta.id = io::filename(pluginPath).toStdString();
        info.path = pluginPath;
        info.enabled = false;
        info.errorCode = code;

        Ret ret = knownPluginsRegister()->registerPlugin(info);
        if (!ret) {
            LOGE() << ret.toString();
        }
    }

    m_progress.finished.send(make_ok());
}

mu::Ret RegisterAudioPluginsScenario::registerPlugin(const io::path_t& pluginPath)
{
    TRACEFUNC;

    IAudioPluginMetaReaderPtr reader = metaReader(pluginPath);
    if (!reader) {
        return make_ret(Ret::Code::UnknownError);
    }

    RetVal<AudioResourceMeta> meta = reader->readMeta(pluginPath);
    if (!meta.ret) {
        LOGE() << meta.ret.toString();
        return meta.ret;
    }

    AudioPluginInfo info;
    info.type = audioPluginTypeFromCategoriesString(meta.val.attributeVal(audio::CATEGORIES_ATTRIBUTE).toStdString());
    info.meta = meta.val;
    info.path = pluginPath;
    info.enabled = true;

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
