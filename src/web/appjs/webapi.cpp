/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "webapi.h"

#ifdef Q_OS_WASM
#include <emscripten/bind.h>
#include <emscripten/val.h>
#endif

#include "global/io/file.h"

#include "log.h"

using namespace muse;
using namespace mu::appjs;

#ifdef Q_OS_WASM
static void callJsWithBytes(const char* fnname, const uint8_t* data, size_t size)
{
    emscripten::val jsArray = emscripten::val::global("Uint8Array").new_(
        emscripten::typed_memory_view(size, data)
        );

    emscripten::val::module_property(fnname)(jsArray);
}

#else

static void callJsWithBytes(const char*, const uint8_t*, size_t)
{
    NOT_SUPPORTED;
}

#endif

WebApi* WebApi::instance()
{
    static WebApi a;

    return &a;
}

void WebApi::init(const muse::modularity::ContextPtr& ctx)
{
    setContext(ctx);

    auto onProjectChanged = [this]() {
        if (m_currentProject) {
            m_currentProject->saveComplited().disconnect(this);
        }

        m_currentProject = projectSession()->project();

        if (m_currentProject) {
            m_currentProject->saveComplited().onReceive(this, [this](const muse::io::path_t& path, project::SaveMode mode) {
                onProjectSaved(path, mode);
            });
        }
    };

    projectSession()->projectChanged().onNotify(this, onProjectChanged);

    onProjectChanged();
}

void WebApi::deinit()
{
    if (m_currentProject) {
        m_currentProject->saveComplited().disconnect(this);
        m_currentProject = nullptr;
    }

    if (projectSession()) {
        projectSession()->projectChanged().disconnect(this);
    }

    setContext(nullptr);
}

void WebApi::load(const void* source, unsigned int len)
{
    LOGI() << source << ", len: " << len;
    //! NOTE Copy, the source is freed by the caller right after this call, but the project is opened later
    ByteArray data(reinterpret_cast<const uint8_t*>(source), len);

    //! NOTE A new file for each load: the opened project keeps its file, and the open scenario
    //! treats the same path as the already opened project
    io::path_t filePath = "/mu/temp/score_" + std::to_string(++m_loadCount) + ".mscz";
    Ret ret = io::File::writeFile(filePath, data);
    if (!ret) {
        LOGE() << "failed write project, path: " << filePath << ", err: " << ret.toString();
        return;
    }

    //! NOTE Single window: close the current project first, otherwise the open scenario opens a new window
    closeProjectScenario()->closeOpenedProject(false).onResolve(this, [this, filePath](const Ret& closeRet) {
        if (!closeRet) {
            LOGW() << "the current project is not closed, err: " << closeRet.toString();
            io::File::remove(filePath);
            return;
        }

        if (!m_projectFilePath.empty()) {
            io::File::remove(m_projectFilePath);
            m_projectFilePath = io::path_t();
        }

        openProjectScenario()->openProject(filePath).onResolve(this, [this, filePath](const Ret& openRet) {
            if (!openRet) {
                LOGE() << "failed open project, path: " << filePath << ", err: " << openRet.toString();
                io::File::remove(filePath);
                return;
            }

            m_projectFilePath = filePath;
        });
    });
}

void WebApi::addSoundFont(const std::string& uri)
{
    soundFontController()->addSoundFont(Uri(uri));
}

void WebApi::startAudioProcessing()
{
    startAudioController()->startAudioProcessing(IApplication::RunMode::GuiApp);
}

void WebApi::onProjectSaved(const muse::io::path_t& path, mu::project::SaveMode)
{
    IF_ASSERT_FAILED(io::File::exists(path)) {
        LOGE() << "file does not exist, path: " << path;
        return;
    }

    ByteArray data;
    Ret ret  = io::File::readFile(path, data);
    if (!ret) {
        LOGE() << "failed read file, path: " << path;
        return;
    }

    callJsWithBytes("onProjectSaved", data.constData(), data.size());
}
