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
#include "interactive.h"

#include <QFileDialog>
#include <QMainWindow>

#include <QMessageBox>
#include <QPushButton>
#include <QMap>
#include <QSpacerItem>
#include <QGridLayout>
#include <QDesktopServices>

#include "log.h"
#include "translation.h"
#include "io/path.h"

using namespace mu;
using namespace mu::framework;

static IInteractive::Result standardDialogResult(const RetVal<Val>& retVal)
{
    if (!retVal.ret) {
        return IInteractive::Result();
    }

    QVariantMap resultMap = retVal.val.toQVariant().toMap();

    int btn = resultMap["buttonId"].toInt();
    bool showAgain = resultMap["showAgain"].toBool();
    return IInteractive::Result(btn, showAgain);
}

IInteractive::Result Interactive::question(const std::string& title, const std::string& text,
                                           const Buttons& buttons,
                                           const Button& def,
                                           const Options& options) const
{
    return question(title, Text(text), buttonDataList(buttons), int(def), options);
}

IInteractive::Result Interactive::question(const std::string& title, const Text& text, const ButtonDatas& btns, int defBtn,
                                           const Options& options) const
{
    return standardDialogResult(provider()->question(title, text, btns, defBtn, options));
}

IInteractive::ButtonData Interactive::buttonData(Button b) const
{
    constexpr bool accent = true;

    switch (b) {
    case IInteractive::Button::NoButton:    return ButtonData(int(b), "");
    case IInteractive::Button::Ok:          return ButtonData(int(b), trc("ui", "OK"));
    case IInteractive::Button::Save:        return ButtonData(int(b), trc("ui", "Save"), accent);
    case IInteractive::Button::SaveAll:     return ButtonData(int(b), trc("ui", "Save All"));
    case IInteractive::Button::DontSave:    return ButtonData(int(b), trc("ui", "Don't save"));
    case IInteractive::Button::Open:        return ButtonData(int(b), trc("ui", "Open"));
    case IInteractive::Button::Yes:         return ButtonData(int(b), trc("ui", "Yes"), accent);
    case IInteractive::Button::YesToAll:    return ButtonData(int(b), trc("ui", "Yes to All"), accent);
    case IInteractive::Button::No:          return ButtonData(int(b), trc("ui", "No"));
    case IInteractive::Button::NoToAll:     return ButtonData(int(b), trc("ui", "No to All"));
    case IInteractive::Button::Abort:       return ButtonData(int(b), trc("ui", "Abort"));
    case IInteractive::Button::Retry:       return ButtonData(int(b), trc("ui", "Retry"));
    case IInteractive::Button::Ignore:      return ButtonData(int(b), trc("ui", "Ignore"));
    case IInteractive::Button::Close:       return ButtonData(int(b), trc("ui", "Close"));
    case IInteractive::Button::Cancel:      return ButtonData(int(b), trc("ui", "Cancel"));
    case IInteractive::Button::Discard:     return ButtonData(int(b), trc("ui", "Discard"));
    case IInteractive::Button::Help:        return ButtonData(int(b), trc("ui", "Help"));
    case IInteractive::Button::Apply:       return ButtonData(int(b), trc("ui", "Apply"));
    case IInteractive::Button::Reset:       return ButtonData(int(b), trc("ui", "Reset"));
    case IInteractive::Button::RestoreDefaults: return ButtonData(int(b), trc("ui", "Restore Defaults"));
    case IInteractive::Button::CustomButton: return ButtonData(int(b), "");
    }

    return ButtonData(int(b), "");
}

IInteractive::Result Interactive::info(const std::string& title, const std::string& text, const ButtonDatas& buttons,
                                       int defBtn,
                                       const Options& options) const
{
    return standardDialogResult(provider()->info(title, text, buttons, defBtn, options));
}

Interactive::Result Interactive::warning(const std::string& title, const std::string& text, const Buttons& buttons, const Button& defBtn,
                                         const Options& options) const
{
    return standardDialogResult(provider()->warning(title, text, buttonDataList(buttons), int(defBtn), options));
}

IInteractive::Result Interactive::warning(const std::string& title, const Text& text, const ButtonDatas& buttons,
                                          int defBtn,
                                          const QFlags<IInteractive::Option>& options) const
{
    return standardDialogResult(provider()->warning(title, text.text, buttons, defBtn, options));
}

IInteractive::Result Interactive::error(const std::string& title, const std::string& text, const Buttons& buttons, const Button& defBtn,
                                        const Options& options) const
{
    return standardDialogResult(provider()->error(title, text, buttonDataList(buttons), int(defBtn), options));
}

IInteractive::Result Interactive::error(const std::string& title, const Text& text, const ButtonDatas& buttons,
                                        int defBtn,
                                        const Options& options) const
{
    return standardDialogResult(provider()->error(title, text.text, buttons, defBtn, options));
}

mu::io::path Interactive::selectOpeningFile(const QString& title, const io::path& dir, const QString& filter)
{
    QString path = QFileDialog::getOpenFileName(nullptr, title, dir.toQString(), filter);
    return path;
}

io::path Interactive::selectSavingFile(const QString& title, const io::path& dir, const QString& filter, bool confirmOverwrite)
{
    QFileDialog::Options options;
    options.setFlag(QFileDialog::DontConfirmOverwrite, !confirmOverwrite);
    QString path = QFileDialog::getSaveFileName(nullptr, title, dir.toQString(), filter, nullptr, options);
    return path;
}

io::path Interactive::selectDirectory(const QString& title, const io::path& dir)
{
    QString path = QFileDialog::getExistingDirectory(nullptr, title, dir.toQString());
    return path;
}

RetVal<Val> Interactive::open(const std::string& uri) const
{
    return open(UriQuery(uri));
}

RetVal<Val> Interactive::open(const UriQuery& uri) const
{
    UriQuery newQuery = uri;
    if (!newQuery.contains("sync")) {
        newQuery.addParam("sync", Val(true));
    }

    return provider()->open(newQuery);
}

RetVal<bool> Interactive::isOpened(const std::string& uri) const
{
    return provider()->isOpened(Uri(uri));
}

RetVal<bool> Interactive::isOpened(const Uri& uri) const
{
    return provider()->isOpened(uri);
}

void Interactive::close(const std::string& uri)
{
    provider()->close(Uri(uri));
}

void Interactive::close(const Uri& uri)
{
    provider()->close(uri);
}

ValCh<Uri> Interactive::currentUri() const
{
    return provider()->currentUri();
}

std::vector<Uri> Interactive::stack() const
{
    return provider()->stack();
}

Ret Interactive::openUrl(const std::string& url) const
{
    QUrl _url(QString::fromStdString(url));
    return QDesktopServices::openUrl(_url);
}

IInteractive::ButtonDatas Interactive::buttonDataList(const Buttons& buttons) const
{
    ButtonDatas result;
    result.reserve(buttons.size());

    for (Button b : buttons) {
        result.push_back(buttonData(b));
    }

    return result;
}
