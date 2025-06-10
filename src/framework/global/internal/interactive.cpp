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

#include <QUrl>

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QMap>
#include <QSpacerItem>
#include <QGridLayout>
#include <QDesktopServices>

#ifdef Q_OS_MAC
#include "platform/macos/macosinteractivehelper.h"
#elif defined(Q_OS_WIN)
#include <QDir>
#include <QProcess>
#include "platform/win/wininteractivehelper.h"
#endif

#include "translation.h"
#include "io/path.h"

#include "log.h"

using namespace muse;

IInteractive::ButtonData Interactive::buttonData(Button b) const
{
    constexpr bool accent = true;

    switch (b) {
    case IInteractive::Button::NoButton:    return ButtonData(int(b), "");
    case IInteractive::Button::Ok:          return ButtonData(int(b), muse::trc("global", "OK"), accent);
    case IInteractive::Button::Save:        return ButtonData(int(b), muse::trc("global", "Save"), accent);
    case IInteractive::Button::SaveAll:     return ButtonData(int(b), muse::trc("global", "Save all"));
    case IInteractive::Button::DontSave:    return ButtonData(int(b), muse::trc("global", "Don’t save"));
    case IInteractive::Button::Open:        return ButtonData(int(b), muse::trc("global", "Open"));
    case IInteractive::Button::Yes:         return ButtonData(int(b), muse::trc("global", "Yes"), accent);
    case IInteractive::Button::YesToAll:    return ButtonData(int(b), muse::trc("global", "Yes to all"), accent);
    case IInteractive::Button::No:          return ButtonData(int(b), muse::trc("global", "No"));
    case IInteractive::Button::NoToAll:     return ButtonData(int(b), muse::trc("global", "No to all"));
    case IInteractive::Button::Abort:       return ButtonData(int(b), muse::trc("global", "Abort"));
    case IInteractive::Button::Retry:       return ButtonData(int(b), muse::trc("global", "Retry"));
    case IInteractive::Button::Ignore:      return ButtonData(int(b), muse::trc("global", "Ignore"));
    case IInteractive::Button::Close:       return ButtonData(int(b), muse::trc("global", "Close"));
    case IInteractive::Button::Cancel:      return ButtonData(int(b), muse::trc("global", "Cancel"));
    case IInteractive::Button::Discard:     return ButtonData(int(b), muse::trc("global", "Discard"));
    case IInteractive::Button::Help:        return ButtonData(int(b), muse::trc("global", "Help"));
    case IInteractive::Button::Apply:       return ButtonData(int(b), muse::trc("global", "Apply"));
    case IInteractive::Button::Reset:       return ButtonData(int(b), muse::trc("global", "Reset"));
    case IInteractive::Button::Continue:    return ButtonData(int(b), muse::trc("global", "Continue"));
    case IInteractive::Button::Next:
    case IInteractive::Button::Back:
    case IInteractive::Button::Select:
    case IInteractive::Button::Clear:
    case IInteractive::Button::Done:
    case IInteractive::Button::RestoreDefaults:
    case IInteractive::Button::CustomButton: break;
    }

    return ButtonData(int(b), "");
}

UriQuery Interactive::makeQuery(const std::string& type, const std::string& contentTitle,
                                const Text& text,
                                const ButtonDatas& buttons, int defBtn,
                                const Options& options, const std::string& dialogTitle) const
{
    auto format = [](IInteractive::TextFormat f) {
        switch (f) {
        case IInteractive::TextFormat::Auto:      return Qt::AutoText;
        case IInteractive::TextFormat::PlainText: return Qt::PlainText;
        case IInteractive::TextFormat::RichText:  return Qt::RichText;
        }
        return Qt::PlainText;
    };

    UriQuery q("muse://interactive/standard");
    q.set("type", type)
    .set("contentTitle", contentTitle)
    .set("text", text.text)
    .set("textFormat", (int)format(text.format))
    .set("detailedText", text.detailedText)
    .set("defaultButtonId", defBtn)
    .set("withIcon", options.testFlag(IInteractive::Option::WithIcon))
    .set("withDontShowAgainCheckBox", options.testFlag(IInteractive::Option::WithDontShowAgainCheckBox))
    .set("dialogTitle", dialogTitle);

    ValList buttonsList;
    ValList customButtonsList;
    if (buttons.empty()) {
        buttonsList.push_back(Val(static_cast<int>(IInteractive::Button::Ok)));
    } else {
        for (const IInteractive::ButtonData& buttonData: buttons) {
            ValMap customButton;
            customButton["text"] = Val(buttonData.text);
            customButton["buttonId"] = Val(buttonData.btn);
            customButton["role"] = Val(static_cast<int>(buttonData.role));
            customButton["isAccent"] = Val(buttonData.accent);
            customButton["isLeftSide"] = Val(buttonData.leftSide);
            customButtonsList.push_back(Val(customButton));
        }
    }

    q.set("buttons", Val(buttonsList))
    .set("customButtons", Val(customButtonsList));

    return q;
}

IInteractive::Result Interactive::makeResult(const Val& val) const
{
    QVariantMap resultMap = val.toQVariant().toMap();
    int btn = resultMap["buttonId"].toInt();
    bool showAgain = resultMap["showAgain"].toBool();
    return IInteractive::Result(btn, showAgain);
}

async::Promise<IInteractive::Result> Interactive::openStandardAsync(const std::string& type, const std::string& contentTitle,
                                                                    const Text& text,
                                                                    const ButtonDatas& buttons, int defBtn,
                                                                    const Options& options, const std::string& dialogTitle)
{
    UriQuery q = makeQuery(type, contentTitle, text, buttons, defBtn, options, dialogTitle);

    async::Promise<Val> promise = provider()->openAsync(q);

    return async::make_promise<Result>([promise, this](auto resolve, auto reject) {
        async::Promise<Val> mut = promise;
        mut.onResolve(this, [this, resolve](const Val& val) {
            (void)resolve(makeResult(val));
        }).onReject(this, [resolve, reject](int code, const std::string& err) {
            //! NOTE To simplify writing the handlers
            (void)resolve(IInteractive::Result((int)IInteractive::Button::Cancel, false));
            (void)reject(code, err);
        });
        return async::Promise<IInteractive::Result>::Result::unchecked();
    });
}

IInteractive::Result Interactive::openStandardSync(const std::string& type, const std::string& contentTitle,
                                                   const Text& text,
                                                   const ButtonDatas& buttons, int defBtn,
                                                   const Options& options, const std::string& dialogTitle)
{
    UriQuery q = makeQuery(type, contentTitle, text, buttons, defBtn, options, dialogTitle);
    RetVal<Val> rv = provider()->openSync(q);

    if (rv.ret) {
        return makeResult(rv.val);
    } else {
        return IInteractive::Result((int)IInteractive::Button::Cancel);
    }
}

IInteractive::Result Interactive::questionSync(const std::string& contentTitle, const Text& text,
                                               const ButtonDatas& buttons, int defBtn,
                                               const Options& options, const std::string& dialogTitle)
{
    return openStandardSync("QUESTION", contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> Interactive::question(const std::string& contentTitle, const Text& text,
                                                           const ButtonDatas& buttons, int defBtn,
                                                           const Options& options, const std::string& dialogTitle)
{
    return openStandardAsync("QUESTION", contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result Interactive::infoSync(const std::string& contentTitle, const Text& text,
                                           const ButtonDatas& buttons, int defBtn,
                                           const Options& options, const std::string& dialogTitle)
{
    return openStandardSync("INFO", contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> Interactive::info(const std::string& contentTitle, const Text& text,
                                                       const ButtonDatas& buttons, int defBtn,
                                                       const Options& options, const std::string& dialogTitle)
{
    return openStandardAsync("INFO", contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result Interactive::warningSync(const std::string& contentTitle, const Text& text,
                                              const ButtonDatas& buttons, int defBtn,
                                              const Options& options, const std::string& dialogTitle)
{
    return openStandardSync("WARNING", contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> Interactive::warning(const std::string& contentTitle, const Text& text,
                                                          const ButtonDatas& buttons, int defBtn,
                                                          const Options& options, const std::string& dialogTitle)
{
    return openStandardAsync("WARNING", contentTitle, text, buttons, defBtn, options, dialogTitle);
}

IInteractive::Result Interactive::errorSync(const std::string& contentTitle, const Text& text,
                                            const ButtonDatas& buttons, int defBtn,
                                            const Options& options, const std::string& dialogTitle)
{
    return openStandardSync("ERROR", contentTitle, text, buttons, defBtn, options, dialogTitle);
}

async::Promise<IInteractive::Result> Interactive::error(const std::string& contentTitle, const Text& text,
                                                        const ButtonDatas& buttons, int defBtn,
                                                        const Options& options, const std::string& dialogTitle)
{
    return openStandardAsync("ERROR", contentTitle, text, buttons, defBtn, options, dialogTitle);
}

void Interactive::showProgress(const std::string& title, Progress* progress)
{
    Uri uri("muse://interactive/progress");
    QVariantMap params;
    params["title"] = QString::fromStdString(title);
    params["progress"] = QVariant::fromValue(progress);

    provider()->openAsync(uri, params);
}

#ifdef Q_OS_LINUX
static UriQuery makeSelectFileQuery(int mode, const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter,
                                    bool confirmOverwrite)
{
    UriQuery q("muse://interactive/selectfile");
    q.set("title", title);

    ValList filterList;
    for (const std::string& f : filter) {
        filterList.push_back(Val(f));
    }

    q.set("nameFilters", filterList);
    q.set("selectExisting", true);
    // see QQuickPlatformFileDialog::FileMode::OpenFile
    q.set("fileMode", mode);
    q.set("folder", QUrl::fromLocalFile(dir.toQString()).toLocalFile().toStdString());

    if (!confirmOverwrite) {
        q.set("options", QFileDialog::DontConfirmOverwrite);
    }

    return q;
}

#endif

#ifndef Q_OS_LINUX
static QString filterToString(const std::vector<std::string>& filter)
{
    QStringList result;
    for (const std::string& nameFilter : filter) {
        result << QString::fromStdString(nameFilter);
    }

    return result.join(";;");
}

#endif

async::Promise<io::path_t> Interactive::selectOpeningFile(const std::string& title, const io::path_t& dir,
                                                          const std::vector<std::string>& filter)
{
#ifndef Q_OS_LINUX
    return async::make_promise<io::path_t>([title, dir, filter](auto resolve, auto reject) {
        QFileDialog* dlg = new QFileDialog(nullptr, QString::fromStdString(title), dir.toQString(), filterToString(filter));

        dlg->setFileMode(QFileDialog::ExistingFile);

        QObject::connect(dlg, &QFileDialog::finished, [dlg, resolve, reject](int result) {
            dlg->deleteLater();

            QStringList files = dlg->selectedFiles();

            if (result != QDialog::Accepted || files.isEmpty()) {
                (void)reject((int)Ret::Code::Cancel, "cancel");
                return;
            }

            QString file = files.first();
            (void)resolve(file);
        });

        dlg->open();

        return async::Promise<io::path_t>::Result::unchecked();
    }, async::PromiseType::AsyncByBody);

#else

    // see QQuickPlatformFileDialog::FileMode::OpenFile
    int mode = 0;
    UriQuery q = makeSelectFileQuery(mode, title, dir, filter, false);

    async::Promise<Val> promise = provider()->openAsync(q);

    return async::make_promise<io::path_t>([promise, this](auto resolve, auto reject) {
        async::Promise<Val> mut = promise;
        mut.onResolve(this, [resolve](const Val& val) {
            (void)resolve(QUrl::fromUserInput(val.toQString()).toLocalFile());
        }).onReject(this, [resolve, reject](int code, const std::string& err) {
            (void)reject(code, err);
        });
        return async::Promise<io::path_t>::Result::unchecked();
    }, async::PromiseType::AsyncByBody);
#endif
}

io::path_t Interactive::selectOpeningFileSync(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter)
{
#ifndef Q_OS_LINUX
    QString result = QFileDialog::getOpenFileName(nullptr, QString::fromStdString(title), dir.toQString(), filterToString(filter));
    return result;
#else

    // see QQuickPlatformFileDialog::FileMode::OpenFile
    int mode = 0;
    UriQuery q = makeSelectFileQuery(mode, title, dir, filter, false);

    RetVal<Val> rv = provider()->openSync(q);
    if (!rv.ret) {
        return io::path_t();
    }

    return QUrl::fromUserInput(rv.val.toQString()).toLocalFile();
#endif
}

io::path_t Interactive::selectSavingFileSync(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter,
                                             bool confirmOverwrite)
{
#ifndef Q_OS_LINUX
    QFileDialog::Options options;
    options.setFlag(QFileDialog::DontConfirmOverwrite, !confirmOverwrite);
    QString result = QFileDialog::getSaveFileName(nullptr, QString::fromStdString(title), dir.toQString(), filterToString(
                                                      filter), nullptr, options);
    return result;
#else

    // see QQuickPlatformFileDialog::FileMode::SaveFile
    int mode = 2;
    UriQuery q = makeSelectFileQuery(mode, title, dir, filter, confirmOverwrite);

    RetVal<Val> rv = provider()->openSync(q);
    if (!rv.ret) {
        return io::path_t();
    }

    return QUrl::fromUserInput(rv.val.toQString()).toLocalFile();
#endif
}

io::path_t Interactive::selectDirectory(const std::string& title, const io::path_t& dir)
{
#ifndef Q_OS_LINUX
    QString result = QFileDialog::getExistingDirectory(nullptr, QString::fromStdString(title), dir.toQString());
    return result;
#else

    UriQuery q("muse://interactive/selectdir");
    q.set("title", title);
    q.set("folder", QUrl::fromLocalFile(dir.toQString()).toLocalFile().toStdString());

    RetVal<Val> rv = provider()->openSync(q);
    if (!rv.ret) {
        return io::path_t();
    }

    return QUrl::fromUserInput(rv.val.toQString()).toLocalFile();
#endif
}

io::paths_t Interactive::selectMultipleDirectories(const std::string& title, const io::path_t& dir, const io::paths_t& selectedDirectories)
{
    UriQuery q("muse://interactive/selectmultipledirectories");
    q.set("title", title)
    .set("selectedDirectories", io::pathsToString(selectedDirectories))
    .set("startDir", dir.toStdString());

    RetVal<Val> result = openSync(q);
    if (!result.ret) {
        return selectedDirectories;
    }

    return io::pathsFromString(result.val.toString());
}

QColor Interactive::selectColor(const QColor& color, const std::string& title)
{
    shortcutsRegister()->setActive(false);
    QColor selectColor = provider()->selectColor(color, title).val;
    shortcutsRegister()->setActive(true);
    return selectColor;
}

bool Interactive::isSelectColorOpened() const
{
    return provider()->isSelectColorOpened();
}

RetVal<Val> Interactive::openSync(const UriQuery& uri)
{
    UriQuery newQuery = uri;
    if (!newQuery.contains("sync")) {
        newQuery.addParam("sync", Val(true));
    }

    return provider()->openSync(newQuery);
}

async::Promise<Val> Interactive::open(const UriQuery& uri)
{
    return provider()->openAsync(uri);
}

RetVal<bool> Interactive::isOpened(const UriQuery& uri) const
{
    return provider()->isOpened(uri);
}

RetVal<bool> Interactive::isOpened(const Uri& uri) const
{
    return provider()->isOpened(uri);
}

async::Channel<Uri> Interactive::opened() const
{
    return provider()->opened();
}

void Interactive::raise(const UriQuery& uri)
{
    provider()->raise(uri);
}

void Interactive::close(const UriQuery& uri)
{
    provider()->close(uri);
}

void Interactive::close(const Uri& uri)
{
    provider()->close(uri);
}

void Interactive::closeAllDialogs()
{
    provider()->closeAllDialogs();
}

ValCh<Uri> Interactive::currentUri() const
{
    return provider()->currentUri();
}

RetVal<bool> Interactive::isCurrentUriDialog() const
{
    return provider()->isCurrentUriDialog();
}

std::vector<Uri> Interactive::stack() const
{
    return provider()->stack();
}

Ret Interactive::openUrl(const std::string& url) const
{
    return openUrl(QUrl(QString::fromStdString(url)));
}

Ret Interactive::openUrl(const QUrl& url) const
{
    return QDesktopServices::openUrl(url);
}

Ret Interactive::isAppExists(const std::string& appIdentifier) const
{
#ifdef Q_OS_MACOS
    return MacOSInteractiveHelper::isAppExists(appIdentifier);
#else
    NOT_IMPLEMENTED;
    UNUSED(appIdentifier);
    return false;
#endif
}

Ret Interactive::canOpenApp(const Uri& uri) const
{
#ifdef Q_OS_MACOS
    return MacOSInteractiveHelper::canOpenApp(uri);
#else
    NOT_IMPLEMENTED;
    UNUSED(uri);
    return false;
#endif
}

async::Promise<Ret> Interactive::openApp(const Uri& uri) const
{
#ifdef Q_OS_MACOS
    return MacOSInteractiveHelper::openApp(uri);
#elif defined(Q_OS_WIN)
    return WinInteractiveHelper::openApp(uri);
#else
    UNUSED(uri);
    return async::Promise<Ret>([](auto, auto reject) {
        Ret ret = make_ret(Ret::Code::NotImplemented);
        return reject(ret.code(), ret.text());
    });
#endif
}

Ret Interactive::revealInFileBrowser(const io::path_t& filePath) const
{
#ifdef Q_OS_MACOS
    if (MacOSInteractiveHelper::revealInFinder(filePath)) {
        return true;
    }
#elif defined(Q_OS_WIN)
    QString command = QLatin1String("explorer /select,%1").arg(QDir::toNativeSeparators(filePath.toQString()));
    if (QProcess::startDetached(command, QStringList())) {
        return true;
    }
#endif
    io::path_t dirPath = io::dirpath(filePath);
    return openUrl(QUrl::fromLocalFile(dirPath.toQString()));
}
