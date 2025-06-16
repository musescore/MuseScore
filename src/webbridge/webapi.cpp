/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
using namespace mu::webbridge;

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

void WebApi::init()
{
    auto onProjectChanged = [this]() {
        if (m_currentProject) {
            m_currentProject->saveComplited().resetOnReceive(this);
        }

        m_currentProject = globalContext()->currentProject();

        if (m_currentProject) {
            m_currentProject->saveComplited().onReceive(this, [this](const muse::io::path_t& path, project::SaveMode mode) {
                onProjectSaved(path, mode);
            });
        }
    };

    globalContext()->currentProjectChanged().onNotify(this, onProjectChanged);

    onProjectChanged();
}

void WebApi::deinit()
{
    if (m_currentProject) {
        m_currentProject->saveComplited().resetOnReceive(this);
    }
}

void WebApi::onclickTest1(int num)
{
    LOGI() << "num: " << num;
    interactive()->info("onclickTest1", "Hey!");
}

void WebApi::load(const void* source, unsigned int len)
{
    LOGI() << source << ", len: " << len;
    ByteArray data = ByteArray::fromRawData(reinterpret_cast<const char*>(source), len);
    io::path_t tempFilePath = "/mu/temp/current.mscz";

    //! NOTE Remove last previous
    io::File::remove(tempFilePath);

    //! NOTE Write new project
    io::File::writeFile(tempFilePath, data);

    dispatcher()->dispatch("file-open", actions::ActionData::make_arg1(QUrl::fromLocalFile(tempFilePath.toQString())));
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
