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
#ifndef MUSE_AUTOBOT_AUTOBOTINTERACTIVE_H
#define MUSE_AUTOBOT_AUTOBOTINTERACTIVE_H

#include <memory>

#include "iinteractive.h"

namespace muse::autobot {
class AutobotInteractive : public IInteractive
{
public:
    AutobotInteractive() = default;

    void setRealInteractive(std::shared_ptr<IInteractive> real);
    std::shared_ptr<IInteractive> realInteractive() const;

    Result questionSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                        const Options& options = {}, const std::string& dialogTitle = "") override;

    async::Promise<Result> question(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons,
                                    int defBtn = int(Button::NoButton), const Options& options = {},
                                    const std::string& dialogTitle = "") override;

    ButtonData buttonData(Button b) const override;

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
                     const Options& options = {}, const std::string& dialogTitle = "") override;

    async::Promise<Result> error(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons = {},
                                 int defBtn = int(Button::NoButton), const Options& options = {},
                                 const std::string& dialogTitle = "") override;

    // progress
    Ret showProgress(const std::string& title, Progress* progress) const override;

    // files
    io::path_t selectOpeningFile(const QString& title, const io::path_t& dir, const std::vector<std::string>& filter) override;
    io::path_t selectSavingFile(const QString& title, const io::path_t& dir, const std::vector<std::string>& filter,
                                bool confirmOverwrite = true) override;

    // dirs
    io::path_t selectDirectory(const QString& title, const io::path_t& dir) override;
    io::paths_t selectMultipleDirectories(const QString& title, const io::path_t& dir, const io::paths_t& initialDirectories) override;

    // color
    QColor selectColor(const QColor& color = Qt::white, const QString& title = "") override;
    bool isSelectColorOpened() const override;

    // custom
    RetVal<Val> open(const std::string& uri) const override;
    RetVal<Val> open(const Uri& uri) const override;
    RetVal<Val> open(const UriQuery& uri) const override;
    async::Promise<Val> openAsync(const UriQuery& uri) override;
    RetVal<bool> isOpened(const std::string& uri) const override;
    RetVal<bool> isOpened(const Uri& uri) const override;
    RetVal<bool> isOpened(const UriQuery& uri) const override;
    async::Channel<Uri> opened() const override;

    void raise(const UriQuery& uri) override;

    void close(const std::string& uri) override;
    void close(const Uri& uri) override;
    void close(const UriQuery& uri) override;
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

    // AutobotInteractive
    io::path_t selectedFilePath() const; // last selected file path

private:

    std::shared_ptr<IInteractive> m_real = nullptr;

    io::path_t m_selectedFilePath;
};

using AutobotInteractivePtr = std::shared_ptr<AutobotInteractive>;
}

#endif // MUSE_AUTOBOT_AUTOBOTINTERACTIVE_H
