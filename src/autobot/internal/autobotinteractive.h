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
#ifndef MU_AUTOBOT_AUTOBOTINTERACTIVE_H
#define MU_AUTOBOT_AUTOBOTINTERACTIVE_H

#include <memory>

#include "iinteractive.h"

namespace mu::autobot {
class AutobotInteractive : public framework::IInteractive
{
public:
    AutobotInteractive() = default;

    void setRealInteractive(std::shared_ptr<framework::IInteractive> real);
    std::shared_ptr<framework::IInteractive> realInteractive() const;

    Result question(const std::string& title, const std::string& text, const Buttons& buttons, const Button& def = Button::NoButton,
                    const Options& options = {}) const override;

    Result question(const std::string& title, const Text& text, const ButtonDatas& buttons, int defBtn = int(Button::NoButton),
                    const Options& options = {}) const override;

    ButtonData buttonData(Button b) const override;

    // info
    Result info(const std::string& title, const std::string& text, const ButtonDatas& buttons = {}, int defBtn = int(Button::NoButton),
                const Options& options = {}) const override;

    // warning
    Result warning(const std::string& title, const std::string& text, const Buttons& buttons = {}, const Button& def = Button::NoButton,
                   const Options& options = {}) const override;

    Result warning(const std::string& title, const Text& text, const ButtonDatas& buttons = {}, int defBtn = int(Button::NoButton),
                   const Options& options = {}) const override;

    // error
    Result error(const std::string& title, const std::string& text, const Buttons& buttons = {}, const Button& def = Button::NoButton,
                 const Options& options = {}) const override;

    Result error(const std::string& title, const Text& text, const ButtonDatas& buttons = {}, int defBtn = int(Button::NoButton),
                 const Options& options = {}) const override;

    // files
    io::path selectOpeningFile(const QString& title, const io::path& dir, const QString& filter) override;
    io::path selectSavingFile(const QString& title, const io::path& dir, const QString& filter, bool confirmOverwrite = true) override;

    // dirs
    io::path selectDirectory(const QString& title, const io::path& dir) override;

    // custom
    RetVal<Val> open(const std::string& uri) const override;
    RetVal<Val> open(const Uri& uri) const override;
    RetVal<Val> open(const UriQuery& uri) const override;
    RetVal<bool> isOpened(const std::string& uri) const override;
    RetVal<bool> isOpened(const Uri& uri) const override;
    RetVal<bool> isOpened(const UriQuery& uri) const override;
    async::Channel<Uri> opened() const override;

    void raise(const UriQuery& uri) override;

    void close(const std::string& uri) override;
    void close(const Uri& uri) override;

    ValCh<Uri> currentUri() const override;
    std::vector<Uri> stack() const override;

    Ret openUrl(const std::string& url) const override;
    Ret openUrl(const QUrl& url) const override;

    Ret revealInFileBrowser(const io::path& filePath) const override;

    // AutobotInteractive
    io::path selectedFilePath() const; // last selected file path

private:

    std::shared_ptr<framework::IInteractive> m_real = nullptr;

    io::path m_selectedFilePath;
};

using AutobotInteractivePtr = std::shared_ptr<AutobotInteractive>;
}

#endif // MU_AUTOBOT_AUTOBOTINTERACTIVE_H
