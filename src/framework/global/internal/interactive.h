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
#ifndef MUSE_GLOBAL_INTERACTIVE_H
#define MUSE_GLOBAL_INTERACTIVE_H

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "ui/iinteractiveprovider.h"
#include "ui/imainwindow.h"

#include "../iinteractive.h"
#include "shortcuts/ishortcutsregister.h"

namespace muse {
class Interactive : public IInteractive, public Injectable, public async::Asyncable
{
    Inject<muse::ui::IInteractiveProvider> provider = { this };
    Inject<muse::ui::IMainWindow> mainWindow = { this };
    Inject<shortcuts::IShortcutsRegister> shortcutsRegister = { this };

public:

    Interactive(const muse::modularity::ContextPtr& ctx)
        : Injectable(ctx) {}

    ButtonData buttonData(Button b) const override;

    // question
    Result questionSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                        const Options& options = {}, const std::string& dialogTitle = "") override;

    async::Promise<Result> question(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons,
                                    int defBtn = int(Button::NoButton), const Options& options = {},
                                    const std::string& dialogTitle = "") override;

    // info
    Result infoSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                    const Options& options = {}, const std::string& dialogTitle = "") override;

    async::Promise<Result> info(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                int defBtn = int(Button::NoButton), const Options& options = {},
                                const std::string& dialogTitle = "") override;

    // warning
    Result warningSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                       const Options& options = {}, const std::string& dialogTitle = "") override;

    async::Promise<Result> warning(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                   int defBtn = int(Button::NoButton), const Options& options = {},
                                   const std::string& dialogTitle = "") override;

    // error
    Result errorSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                     const Options& options = { WithIcon }, const std::string& dialogTitle = "") override;

    async::Promise<Result> error(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                 int defBtn = int(Button::NoButton), const Options& options = { WithIcon },
                                 const std::string& dialogTitle = "") override;

    // progress
    void showProgress(const std::string& title, Progress* progress) override;

    // files
    async::Promise<io::path_t> selectOpeningFile(const std::string& title, const io::path_t& dir,
                                                 const std::vector<std::string>& filter) override;
    io::path_t selectOpeningFileSync(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter) override;
    io::path_t selectSavingFileSync(const std::string& title, const io::path_t& path, const std::vector<std::string>& filter,
                                    bool confirmOverwrite = true) override;

    // dirs
    io::path_t selectDirectory(const std::string& title, const io::path_t& dir) override;
    io::paths_t selectMultipleDirectories(const std::string& title, const io::path_t& dir, const io::paths_t& selectedDirectories) override;

    // color
    QColor selectColor(const QColor& color = Qt::white, const std::string& title = "") override;
    bool isSelectColorOpened() const override;

    // custom
    RetVal<Val> openSync(const UriQuery& uri) override;
    async::Promise<Val> open(const UriQuery& uri) override;
    RetVal<bool> isOpened(const UriQuery& uri) const override;
    RetVal<bool> isOpened(const Uri& uri) const override;
    async::Channel<Uri> opened() const override;

    void raise(const UriQuery& uri) override;

    void close(const UriQuery& uri) override;
    void close(const Uri& uri) override;
    void closeAllDialogs() override;

    ValCh<Uri> currentUri() const override;
    RetVal<bool> isCurrentUriDialog() const override;
    std::vector<Uri> stack() const override;

    Ret openUrl(const std::string& url) const override;
    Ret openUrl(const QUrl& url) const override;

    Ret isAppExists(const std::string& appIdentifier) const override;
    Ret canOpenApp(const Uri& uri) const override;
    async::Promise<Ret> openApp(const Uri& uri) const override;

    Ret revealInFileBrowser(const io::path_t& filePath) const override;

private:
    UriQuery makeQuery(const std::string& type, const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn,
                       const Options& options, const std::string& dialogTitle) const;

    IInteractive::Result makeResult(const Val& val) const;

    async::Promise<IInteractive::Result> openStandardAsync(const std::string& type, const std::string& contentTitle, const Text& text,
                                                           const ButtonDatas& buttons, int defBtn, const Options& options,
                                                           const std::string& dialogTitle);

    IInteractive::Result openStandardSync(const std::string& type, const std::string& contentTitle, const Text& text,
                                          const ButtonDatas& buttons, int defBtn, const Options& options, const std::string& dialogTitle);
};
}

#endif // MUSE_GLOBAL_UIINTERACTIVE_H
