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

IInteractive::Result Interactive::question(const std::string& title, const std::string& text,
                                           const Buttons& buttons,
                                           const Button& def,
                                           const QFlags<Option>& options) const
{
    ButtonDatas datas;
    datas.reserve(buttons.size());
    for (Button b : buttons) {
        datas.push_back(buttonData(b));
    }

    return question(title, Text(text), datas, int(def), options);
}

IInteractive::Result Interactive::question(const std::string& title, const Text& text, const ButtonDatas& btns, int defBtn,
                                           const QFlags<Option>& options) const
{
    Q_UNUSED(options)

    //! NOTE Temporarily, need to replace the qml dialog

    auto format = [](IInteractive::TextFormat f) {
        switch (f) {
        case IInteractive::TextFormat::PlainText: return Qt::PlainText;
        case IInteractive::TextFormat::RichText:  return Qt::RichText;
        }
        return Qt::PlainText;
    };

    QMap<QAbstractButton*, int> btnsMap;
    auto makeButton = [&btnsMap](const ButtonData& b, QWidget* parent) {
        QPushButton* btn = new QPushButton(parent);
        btn->setText(QString::fromStdString(b.text));
        btnsMap[btn] = b.btn;
        return btn;
    };

    QMessageBox msgBox;
    msgBox.setWindowTitle(QString::fromStdString(title));
    msgBox.setText(QString::fromStdString(text.text));
    msgBox.setTextFormat(format(text.format));

    for (const ButtonData& b : btns) {
        QPushButton* btn = makeButton(b, &msgBox);
        msgBox.addButton(btn, QMessageBox::ActionRole);

        if (b.btn == defBtn) {
            msgBox.setDefaultButton(btn);
        }
    }

    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    QSpacerItem* hSpacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(hSpacer, layout->rowCount(), 0, 1, layout->columnCount());

    msgBox.exec();

    QAbstractButton* clickedBtn = msgBox.clickedButton();
    int retBtn = btnsMap.value(clickedBtn);

    return Result(retBtn);
}

IInteractive::ButtonData Interactive::buttonData(Button b) const
{
    switch (b) {
    case IInteractive::Button::NoButton:    return ButtonData(int(b), "");
    case IInteractive::Button::Ok:          return ButtonData(int(b), trc("ui", "OK"));
    case IInteractive::Button::Save:        return ButtonData(int(b), trc("ui", "Save"));
    case IInteractive::Button::SaveAll:     return ButtonData(int(b), trc("ui", "Save All"));
    case IInteractive::Button::Open:        return ButtonData(int(b), trc("ui", "Open"));
    case IInteractive::Button::Yes:         return ButtonData(int(b), trc("ui", "Yes"));
    case IInteractive::Button::YesToAll:    return ButtonData(int(b), trc("ui", "Yes to All"));
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

IInteractive::Result Interactive::info(const std::string& title, const std::string& text,
                                       const QFlags<Option>& options) const
{
    return message(MessageType::Info, title, text, options);
}

IInteractive::Result Interactive::warning(const std::string& title, const std::string& text,
                                          const QFlags<IInteractive::Option>& options) const
{
    return message(MessageType::Warning, title, text, options);
}

IInteractive::Result Interactive::error(const std::string& title, const std::string& text,
                                        const QFlags<Option>& options) const
{
    return message(MessageType::Error, title, text, options);
}

mu::io::path Interactive::selectOpeningFile(const QString& title, const io::path& dir, const QString& filter)
{
    QString path = QFileDialog::getOpenFileName(mainWindow()->qMainWindow(), title, dir.toQString(), filter);
    return path;
}

io::path Interactive::selectSavingFile(const QString& title, const io::path& dir, const QString& filter, bool confirmOverwrite)
{
    QFileDialog::Options options;
    options.setFlag(QFileDialog::DontConfirmOverwrite, !confirmOverwrite);
    QString path = QFileDialog::getSaveFileName(mainWindow()->qMainWindow(), title, dir.toQString(), filter, nullptr, options);
    return path;
}

io::path Interactive::selectDirectory(const QString& title, const io::path& dir)
{
    QString path = QFileDialog::getExistingDirectory(mainWindow()->qMainWindow(), title, dir.toQString());
    return path;
}

RetVal<Val> Interactive::open(const std::string& uri) const
{
    if (uri.find("sync=") != std::string::npos) {
        return provider()->open(UriQuery(uri));
    }

    std::string newUri = uri;
    if (newUri.find("?") == std::string::npos) {
        newUri += "?";
    } else {
        newUri += "&";
    }

    newUri += "sync=true";

    return provider()->open(UriQuery(newUri));
}

RetVal<Val> Interactive::open(const UriQuery& uri) const
{
    return provider()->open(uri);
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

Ret Interactive::openUrl(const std::string& url) const
{
    QUrl _url(QString::fromStdString(url));
    return QDesktopServices::openUrl(_url);
}

IInteractive::Result Interactive::message(MessageType type, const std::string& title, const std::string& text,
                                          const QFlags<Option>& options) const
{
    Q_UNUSED(options)

    //! NOTE Temporarily, need to replace the qml dialog

    auto icon = [](MessageType type) {
        switch (type) {
        case MessageType::Info: return QMessageBox::Information;
        case MessageType::Warning: return QMessageBox::Warning;
        case MessageType::Error: return QMessageBox::Critical;
        }
        return QMessageBox::NoIcon;
    };

    QMessageBox msgBox;
    msgBox.setWindowTitle(QString::fromStdString(title));
    msgBox.setText(QString::fromStdString(text));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIcon(icon(type));

    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    QSpacerItem* hSpacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    layout->addItem(hSpacer, layout->rowCount(), 0, 1, layout->columnCount());

    msgBox.exec();

    return Result();
}
