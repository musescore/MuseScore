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
#pragma once

#include "global/iinteractive.h"

namespace mu::webbridge {
class WebInteractive : public muse::IInteractive
{
public:
    WebInteractive(std::shared_ptr<muse::IInteractive> origin);

    ButtonData buttonData(Button b) const override;

    // question
    Result questionSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                        const Options& options = {}, const std::string& dialogTitle = "") override;

    muse::async::Promise<Result> question(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons,
                                          int defBtn = int(Button::NoButton), const Options& options = {},
                                          const std::string& dialogTitle = "") override;

    // info
    Result infoSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                    const Options& options = {}, const std::string& dialogTitle = "") override;

    muse::async::Promise<Result> info(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                      int defBtn = int(Button::NoButton), const Options& options = {},
                                      const std::string& dialogTitle = "") override;

    // warning
    Result warningSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                       const Options& options = {}, const std::string& dialogTitle = "") override;

    muse::async::Promise<Result> warning(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                         int defBtn = int(Button::NoButton), const Options& options = {},
                                         const std::string& dialogTitle = "") override;

    // error
    Result errorSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                     const Options& options = { WithIcon }, const std::string& dialogTitle = "") override;

    muse::async::Promise<Result> error(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                       int defBtn = int(Button::NoButton), const Options& options = { WithIcon },
                                       const std::string& dialogTitle = "") override;

    // progress
    void showProgress(const std::string& title, muse::Progress* progress) override;

    // files
    muse::async::Promise<muse::io::path_t> selectOpeningFile(const std::string& title, const muse::io::path_t& dir,
                                                             const std::vector<std::string>& filter) override;
    muse::io::path_t selectOpeningFileSync(const std::string& title, const muse::io::path_t& dir,
                                           const std::vector<std::string>& filter) override;
    muse::io::path_t selectSavingFileSync(const std::string& title, const muse::io::path_t& path, const std::vector<std::string>& filter,
                                          bool confirmOverwrite = true) override;

    // dirs
    muse::io::path_t selectDirectory(const std::string& title, const muse::io::path_t& dir) override;
    muse::io::paths_t selectMultipleDirectories(const std::string& title, const muse::io::path_t& dir,
                                                const muse::io::paths_t& selectedDirectories) override;

    // color
    QColor selectColor(const QColor& color = Qt::white, const std::string& title = "") override;
    bool isSelectColorOpened() const override;

    // custom
    muse::RetVal<muse::Val> openSync(const muse::UriQuery& uri) override;
    muse::async::Promise<muse::Val> open(const muse::UriQuery& uri) override;
    muse::RetVal<bool> isOpened(const muse::UriQuery& uri) const override;
    muse::RetVal<bool> isOpened(const muse::Uri& uri) const override;
    muse::async::Channel<muse::Uri> opened() const override;

    void raise(const muse::UriQuery& uri) override;

    void close(const muse::UriQuery& uri) override;
    void close(const muse::Uri& uri) override;
    void closeAllDialogs() override;

    muse::ValCh<muse::Uri> currentUri() const override;
    muse::RetVal<bool> isCurrentUriDialog() const override;
    std::vector<muse::Uri> stack() const override;

    muse::Ret openUrl(const std::string& url) const override;
    muse::Ret openUrl(const QUrl& url) const override;

    muse::Ret isAppExists(const std::string& appIdentifier) const override;
    muse::Ret canOpenApp(const muse::Uri& uri) const override;
    muse::async::Promise<muse::Ret> openApp(const muse::Uri& uri) const override;

    muse::Ret revealInFileBrowser(const muse::io::path_t& filePath) const override;

private:

    std::shared_ptr<muse::IInteractive> m_origin = nullptr;
};
}
