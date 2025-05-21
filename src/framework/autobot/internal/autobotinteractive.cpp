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
#include "autobotinteractive.h"

#include "log.h"

using namespace muse;
using namespace muse::autobot;

void AutobotInteractive::setRealInteractive(std::shared_ptr<IInteractive> real)
{
    m_real = real;
}

std::shared_ptr<IInteractive> AutobotInteractive::realInteractive() const
{
    return m_real;
}

IInteractive::Result AutobotInteractive::questionSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons,
                                                      int defBtn, const Options& options, const std::string& dialogTitle)
{
    return m_real->questionSync(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> AutobotInteractive::questionAsync(const std::string& contentTitle, const Text& text,
                                                                       const ButtonDatas& buttons, int defBtn,
                                                                       const Options& options, const std::string& dialogTitle)
{
    return m_real->questionAsync(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::ButtonData AutobotInteractive::buttonData(Button b) const
{
    return m_real->buttonData(b);
}

IInteractive::Result AutobotInteractive::infoSync(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn,
                                                  const Options& options, const std::string& dialogTitle)
{
    return m_real->infoSync(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> AutobotInteractive::infoAsync(const std::string& contentTitle, const Text& text,
                                                                   const ButtonDatas& buttons, int defBtn,
                                                                   const Options& options, const std::string& dialogTitle)
{
    return m_real->infoAsync(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result AutobotInteractive::warning(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                                                 const Button& def, const Options& options, const std::string& dialogTitle) const
{
    return m_real->warning(contentTitle, text, buttons, def, options, dialogTitle);
}

IInteractive::Result AutobotInteractive::warning(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons,
                                                 int defBtn, const Options& options, const std::string& dialogTitle) const
{
    return m_real->warning(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result AutobotInteractive::warning(const std::string& contentTitle, const Text& text, const std::string& detailedText,
                                                 const ButtonDatas& buttons, int defBtn,
                                                 const Options& options, const std::string& dialogTitle) const
{
    return m_real->warning(contentTitle, text, detailedText, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result AutobotInteractive::error(const std::string& contentTitle, const std::string& text, const Buttons& buttons,
                                               const Button& def, const Options& options, const std::string& dialogTitle) const
{
    return m_real->error(contentTitle, text, buttons, def, options, dialogTitle);
}

IInteractive::Result AutobotInteractive::error(const std::string& contentTitle, const Text& text, const ButtonDatas& buttons,
                                               int defBtn, const Options& options, const std::string& dialogTitle) const
{
    return m_real->error(contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result AutobotInteractive::error(const std::string& contentTitle, const Text& text, const std::string& detailedText,
                                               const ButtonDatas& buttons, int defBtn, const Options& options,
                                               const std::string& dialogTitle) const
{
    return m_real->error(contentTitle, text, detailedText, buttons, defBtn, options, dialogTitle);
}

Ret AutobotInteractive::showProgress(const std::string& title, Progress* progress) const
{
    return m_real->showProgress(title, progress);
}

io::path_t AutobotInteractive::selectOpeningFile(const QString& title, const io::path_t& dir, const std::vector<std::string>& filter)
{
    return m_real->selectOpeningFile(title, dir, filter);
}

io::path_t AutobotInteractive::selectSavingFile(const QString& title, const io::path_t& dir, const std::vector<std::string>& filter,
                                                bool confirmOverwrite)
{
    // return m_real->selectSavingFile(title, dir, filter, confirmOverwrite);
    QStringList filterList;
    for (const std::string& fileFilter : filter) {
        filterList << QString::fromStdString(fileFilter);
    }

    LOGD() << title << " dir:" << dir << ", filter: " << filterList << ", confirmOverwrite: " << confirmOverwrite;
    m_real->open("muse://autobot/selectfile?sync=true&filePath=" + dir.toStdString());
    m_selectedFilePath = dir;
    return m_selectedFilePath;
}

io::path_t AutobotInteractive::selectDirectory(const QString& title, const io::path_t& dir)
{
    return m_real->selectDirectory(title, dir);
}

io::paths_t AutobotInteractive::selectMultipleDirectories(const QString& title, const io::path_t& dir,
                                                          const io::paths_t& initialDirectories)
{
    return m_real->selectMultipleDirectories(title, dir, initialDirectories);
}

QColor AutobotInteractive::selectColor(const QColor& color, const QString& title)
{
    return m_real->selectColor(color, title);
}

bool AutobotInteractive::isSelectColorOpened() const
{
    return m_real->isSelectColorOpened();
}

RetVal<Val> AutobotInteractive::open(const std::string& uri) const
{
    return m_real->open(uri);
}

RetVal<Val> AutobotInteractive::open(const Uri& uri) const
{
    return m_real->open(uri);
}

RetVal<Val> AutobotInteractive::open(const UriQuery& uri) const
{
    return m_real->open(uri);
}

async::Promise<Val> AutobotInteractive::openAsync(const UriQuery& uri)
{
    return m_real->openAsync(uri);
}

RetVal<bool> AutobotInteractive::isOpened(const std::string& uri) const
{
    return m_real->isOpened(uri);
}

RetVal<bool> AutobotInteractive::isOpened(const Uri& uri) const
{
    return m_real->isOpened(uri);
}

RetVal<bool> AutobotInteractive::isOpened(const UriQuery& uri) const
{
    return m_real->isOpened(uri);
}

async::Channel<Uri> AutobotInteractive::opened() const
{
    return m_real->opened();
}

void AutobotInteractive::raise(const UriQuery& uri)
{
    m_real->raise(uri);
}

void AutobotInteractive::close(const std::string& uri)
{
    m_real->close(uri);
}

void AutobotInteractive::close(const Uri& uri)
{
    m_real->close(uri);
}

void AutobotInteractive::close(const UriQuery& uri)
{
    m_real->close(uri);
}

void AutobotInteractive::closeAllDialogs()
{
    m_real->closeAllDialogs();
}

ValCh<Uri> AutobotInteractive::currentUri() const
{
    return m_real->currentUri();
}

RetVal<bool> AutobotInteractive::isCurrentUriDialog() const
{
    return m_real->isCurrentUriDialog();
}

std::vector<Uri> AutobotInteractive::stack() const
{
    return m_real->stack();
}

Ret AutobotInteractive::openUrl(const std::string& url) const
{
    return m_real->openUrl(url);
}

Ret AutobotInteractive::openUrl(const QUrl& url) const
{
    return m_real->openUrl(url);
}

Ret AutobotInteractive::isAppExists(const std::string& appIdentifier) const
{
    return m_real->isAppExists(appIdentifier);
}

Ret AutobotInteractive::canOpenApp(const Uri& uri) const
{
    return m_real->canOpenApp(uri);
}

async::Promise<Ret> AutobotInteractive::openApp(const Uri& uri) const
{
    return m_real->openApp(uri);
}

Ret AutobotInteractive::revealInFileBrowser(const io::path_t& filePath) const
{
    return m_real->revealInFileBrowser(filePath);
}

io::path_t AutobotInteractive::selectedFilePath() const
{
    return m_selectedFilePath;
}
