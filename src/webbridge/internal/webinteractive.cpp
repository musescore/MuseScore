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
#include "webinteractive.h"

#ifdef Q_OS_WASM
#include <emscripten/bind.h>
#endif

#include "global/io/file.h"

#include "log.h"

using namespace muse;
using namespace mu::webbridge;

struct FileOpenData {
    std::string lastOpenedFileName;
    std::function<void(const std::string&, const ByteArray&)> callback;
};

static FileOpenData g_fileOpenData;

#ifdef Q_OS_WASM
static void openFileDialog(const emscripten::val& callback)
{
    emscripten::val::global("openFileDialog")(callback);
}

static bool isUint8Array(const emscripten::val& v)
{
    return v.instanceof(emscripten::val::global("Uint8Array"));
}

static void onFileSelected(emscripten::val fileName, emscripten::val fileData)
{
    IF_ASSERT_FAILED(g_fileOpenData.callback) {
        return;
    }

    IF_ASSERT_FAILED(fileName.isString()) {
        return;
    }

    IF_ASSERT_FAILED(isUint8Array(fileData)) {
        return;
    }

    std::string fn = fileName.as<std::string>();
    LOGDA() << "fileName: " << fn;

    size_t length = fileData["length"].as<size_t>();
    ByteArray data(length);

    emscripten::val memoryView = emscripten::val(emscripten::typed_memory_view(
                                                     data.size(),
                                                     data.data()
                                                     ));
    memoryView.call<void>("set", fileData);

    LOGDA() << "length: " << length << ", [0]=" << data.at(0);

    g_fileOpenData.callback(fn, data);
}

EMSCRIPTEN_BINDINGS(MuseScoreStudio) {
    function("openFileDialog", &openFileDialog);
    function("onFileSelected", &onFileSelected);
}
#endif

WebInteractive::WebInteractive(std::shared_ptr<muse::IInteractive> origin)
    : m_origin(origin)
{
}

IInteractive::ButtonData WebInteractive::buttonData(Button b) const
{
    return m_origin->buttonData(b);
}

IInteractive::Result WebInteractive::questionSync(const std::string& contentTitle, const Text& text,
                                                  const ButtonDatas& buttons, int defBtn,
                                                  const Options& options, const std::string& dialogTitle)
{
    return m_origin->questionSync(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> WebInteractive::question(const std::string& contentTitle, const Text& text,
                                                              const ButtonDatas& buttons, int defBtn,
                                                              const Options& options, const std::string& dialogTitle)
{
    return m_origin->question(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result WebInteractive::infoSync(const std::string& contentTitle, const Text& text,
                                              const ButtonDatas& buttons, int defBtn,
                                              const Options& options, const std::string& dialogTitle)
{
    return m_origin->infoSync(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> WebInteractive::info(const std::string& contentTitle, const Text& text,
                                                          const ButtonDatas& buttons, int defBtn,
                                                          const Options& options, const std::string& dialogTitle)
{
    return m_origin->info(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result WebInteractive::warningSync(const std::string& contentTitle, const Text& text,
                                                 const ButtonDatas& buttons, int defBtn,
                                                 const Options& options, const std::string& dialogTitle)
{
    return m_origin->warningSync(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> WebInteractive::warning(const std::string& contentTitle, const Text& text,
                                                             const ButtonDatas& buttons, int defBtn,
                                                             const Options& options, const std::string& dialogTitle)
{
    return m_origin->warning(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result WebInteractive::errorSync(const std::string& contentTitle, const Text& text,
                                               const ButtonDatas& buttons, int defBtn,
                                               const Options& options, const std::string& dialogTitle)
{
    return m_origin->errorSync(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> WebInteractive::error(const std::string& contentTitle, const Text& text,
                                                           const ButtonDatas& buttons, int defBtn,
                                                           const Options& options, const std::string& dialogTitle)
{
    return m_origin->error(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

void WebInteractive::showProgress(const std::string& title, Progress* progress)
{
    m_origin->showProgress(title, progress);
}

async::Promise<io::path_t> WebInteractive::selectOpeningFile(const std::string& title, const io::path_t& dir,
                                                             const std::vector<std::string>& filter)
{
#ifdef Q_OS_WASM
    return async::make_promise<io::path_t>([title, dir, filter](auto resolve, auto reject) {
        IF_ASSERT_FAILED(!g_fileOpenData.callback) {
            LOGE() << "Opening file already open";
            return reject((int)Ret::Code::InternalError, "Opening file already open");
        }

        if (!g_fileOpenData.lastOpenedFileName.empty()) {
            io::File::remove(g_fileOpenData.lastOpenedFileName);
            g_fileOpenData.lastOpenedFileName.clear();
        }

        g_fileOpenData.callback = [resolve, reject](const std::string& fileName, const ByteArray& data) {
            if (fileName.empty()) {
                (void)reject((int)Ret::Code::Cancel, "Cancel");
                return;
            }

            std::string fullFN = "/interactive/" + fileName;

            g_fileOpenData.lastOpenedFileName = fullFN;

            //! NOTE Write to mem FS
            io::File::writeFile(fullFN, data);
            LOGDA() << "writeFile: " << fullFN << ", size: " << data.size();

            (void)resolve(fullFN);
            g_fileOpenData.callback = nullptr;
        };

        openFileDialog(emscripten::val::module_property("onFileSelected"));

        return async::Promise<io::path_t>::Result::unchecked();
    }, async::PromiseType::AsyncByBody);
#else
    return m_origin->selectOpeningFile(title, dir, filter);
#endif
}

io::path_t WebInteractive::selectOpeningFileSync(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter)
{
#ifdef Q_OS_WASM
    UNUSED(title);
    UNUSED(dir);
    UNUSED(filter);
    NOT_SUPPORTED;
    return io::path_t();
#else
    return m_origin->selectOpeningFileSync(title, dir, filter);
#endif
}

io::path_t WebInteractive::selectSavingFileSync(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter,
                                                bool confirmOverwrite)
{
#ifdef Q_OS_WASM
    UNUSED(title);
    UNUSED(dir);
    UNUSED(filter);
    UNUSED(confirmOverwrite);
    NOT_SUPPORTED;
    return io::path_t();
#else
    return m_origin->selectSavingFileSync(title, dir, filter, confirmOverwrite);
#endif
}

io::path_t WebInteractive::selectDirectory(const std::string& title, const io::path_t& dir)
{
#ifdef Q_OS_WASM
    UNUSED(title);
    UNUSED(dir);
    NOT_SUPPORTED;
    return io::path_t();
#else
    return m_origin->selectDirectory(title, dir);
#endif
}

io::paths_t WebInteractive::selectMultipleDirectories(const std::string& title, const io::path_t& dir,
                                                      const io::paths_t& selectedDirectories)
{
#ifdef Q_OS_WASM
    UNUSED(title);
    UNUSED(dir);
    UNUSED(selectedDirectories);
    NOT_SUPPORTED;
    return io::paths_t();
#else
    return m_origin->selectMultipleDirectories(title, dir, selectedDirectories);
#endif
}

QColor WebInteractive::selectColor(const QColor& color, const std::string& title)
{
    return m_origin->selectColor(color, title);
}

bool WebInteractive::isSelectColorOpened() const
{
    return m_origin->isSelectColorOpened();
}

RetVal<Val> WebInteractive::openSync(const UriQuery& uri)
{
    return m_origin->openSync(uri);
}

async::Promise<Val> WebInteractive::open(const UriQuery& uri)
{
    return m_origin->open(uri);
}

RetVal<bool> WebInteractive::isOpened(const UriQuery& uri) const
{
    return m_origin->isOpened(uri);
}

RetVal<bool> WebInteractive::isOpened(const Uri& uri) const
{
    return m_origin->isOpened(uri);
}

async::Channel<Uri> WebInteractive::opened() const
{
    return m_origin->opened();
}

void WebInteractive::raise(const UriQuery& uri)
{
    m_origin->raise(uri);
}

void WebInteractive::close(const UriQuery& uri)
{
    m_origin->close(uri);
}

void WebInteractive::close(const Uri& uri)
{
    m_origin->close(uri);
}

void WebInteractive::closeAllDialogs()
{
    m_origin->closeAllDialogs();
}

ValCh<Uri> WebInteractive::currentUri() const
{
    return m_origin->currentUri();
}

RetVal<bool> WebInteractive::isCurrentUriDialog() const
{
    return m_origin->isCurrentUriDialog();
}

std::vector<Uri> WebInteractive::stack() const
{
    return m_origin->stack();
}

Ret WebInteractive::openUrl(const std::string& url) const
{
    return openUrl(QUrl(QString::fromStdString(url)));
}

Ret WebInteractive::openUrl(const QUrl& url) const
{
    return m_origin->openUrl(url);
}

Ret WebInteractive::isAppExists(const std::string& appIdentifier) const
{
    return m_origin->isAppExists(appIdentifier);
}

Ret WebInteractive::canOpenApp(const Uri& uri) const
{
    return m_origin->canOpenApp(uri);
}

async::Promise<Ret> WebInteractive::openApp(const Uri& uri) const
{
    return m_origin->openApp(uri);
}

Ret WebInteractive::revealInFileBrowser(const io::path_t& filePath) const
{
    return m_origin->revealInFileBrowser(filePath);
}
