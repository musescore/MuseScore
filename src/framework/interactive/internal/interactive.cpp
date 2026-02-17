/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include <QColorDialog>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include <QGuiApplication>
#include <QMetaProperty>
#include <QMetaType>
#include <QUrl>
#include <QWidget>
#include <QWindow>

#ifdef Q_OS_MAC
#include "platform/macos/macosinteractivehelper.h"
#elif defined(Q_OS_WIN)
#include <QDir>
#include <QProcess>
#include "platform/win/wininteractivehelper.h"
#endif

#include "async/async.h"
#include "io/path.h"
#include "translation.h"

#include "diagnostics/diagnosticutils.h"

#include "widgetdialogadapter.h"

#include "muse_framework_config.h"

#include "log.h"

using namespace muse;
using namespace muse::interactive;
using namespace muse::async;

// === QmlLaunchData ===

QmlLaunchData::QmlLaunchData(QObject* parent)
    : QObject(parent)
{
}

QVariant QmlLaunchData::value(const QString& key) const
{
    return m_data.value(key);
}

void QmlLaunchData::setValue(const QString& key, const QVariant& val)
{
    m_data[key] = val;
}

QVariant QmlLaunchData::data() const
{
    return m_data;
}

// === Interactive ===

Interactive::Interactive(const muse::modularity::ContextPtr& ctx)
    : QObject(), Contextable(ctx)
{
    connect(qApp, &QGuiApplication::focusWindowChanged, this, [this](QWindow* window) {
        raiseWindowInStack(window);
    });

    connect(qApp, &QGuiApplication::focusObjectChanged, this, [this](QObject* obj) {
        auto widget = dynamic_cast<QWidget*>(obj);

        if (widget && widget->isWindow()) {
            raiseWindowInStack(widget);
        }
    });
}

void Interactive::raiseWindowInStack(QObject* newActiveWindow)
{
    if (!newActiveWindow || m_stack.isEmpty() || m_stack.top().window == newActiveWindow) {
        return;
    }

    for (int i = 0; i < m_stack.size(); ++i) {
        bool found = m_stack[i].window == newActiveWindow;
        if (m_stack[i].window && m_stack[i].window->isWidgetType()) {
            found = newActiveWindow->objectName() == (m_stack[i].window->objectName() + "Window");
        }

        if (found) {
            ObjectInfo info = m_stack.takeAt(i);
            m_stack.push(info);
            notifyAboutCurrentUriChanged();
            return;
        }
    }
}

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

    return openAsync(q).then<IInteractive::Result>(
        this,
        [this](const Val& val, auto resolve, auto /*reject*/) {
        return resolve(makeResult(val));
    },
        [](int code, const std::string& msg, auto resolve, auto reject) {
        //! NOTE To simplify writing the handlers
        (void)resolve(IInteractive::Result((int)IInteractive::Button::Cancel, false));
        return reject(code, msg);
    });
}

IInteractive::Result Interactive::openStandardSync(const std::string& type, const std::string& contentTitle,
                                                   const Text& text,
                                                   const ButtonDatas& buttons, int defBtn,
                                                   const Options& options, const std::string& dialogTitle)
{
    UriQuery q = makeQuery(type, contentTitle, text, buttons, defBtn, options, dialogTitle);
    RetVal<Val> rv = openSync(q);

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

void Interactive::showProgress(const std::string& title, Progress progress)
{
    Uri uri("muse://interactive/progress");
    QVariantMap params;
    params["title"] = QString::fromStdString(title);
    params["progress"] = QVariant::fromValue(progress);

    openAsync(uri, params);
}

#ifdef Q_OS_LINUX
// see QQuickPlatformFileDialog::FileMode
enum class FileDialogMode {
    OpenFile = 0,
    SaveFile = 2
};

static UriQuery makeSelectFileQuery(FileDialogMode mode, const std::string& title, const io::path_t& current,
                                    const std::vector<std::string>& filter, const int options = 0)
{
    UriQuery q("muse://interactive/selectfile");
    q.set("title", title);

    ValList filterList;
    for (const std::string& f : filter) {
        filterList.push_back(Val(f));
    }

    q.set("nameFilters", filterList);
    q.set("fileMode", static_cast<int>(mode));
    q.set("options", options);
    if (mode == FileDialogMode::OpenFile) {
        q.set("selectExisting", true);
        q.set("folder", QUrl::fromLocalFile(current.toQString()).toString().toStdString());
    } else if (mode == FileDialogMode::SaveFile) {
        q.set("currentFile", QUrl::fromLocalFile(current.toQString()).toString().toStdString());
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
                Ret ret = muse::make_ret(Ret::Code::Cancel);
                (void)reject(ret.code(), ret.text());
                return;
            }

            QString file = files.first();
            (void)resolve(file);
        });

        dlg->open();

        return async::Promise<io::path_t>::Result::unchecked();
    }, async::PromiseType::AsyncByBody);

#else

    UriQuery q = makeSelectFileQuery(FileDialogMode::OpenFile, title, dir, filter);

    async::Promise<Val> promise = openAsync(q);

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

io::path_t Interactive::selectOpeningFileSync(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter,
                                              const int options)
{
#ifndef Q_OS_LINUX
    const QFileDialog::Options qoptions = QFileDialog::Options::fromInt(options);
    QString result = QFileDialog::getOpenFileName(nullptr, QString::fromStdString(title), dir.toQString(), filterToString(
                                                      filter), nullptr, qoptions);
    return result;
#else
    UriQuery q = makeSelectFileQuery(FileDialogMode::OpenFile, title, dir, filter, options);

    RetVal<Val> rv = openSync(q);
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

    UriQuery q
        = makeSelectFileQuery(FileDialogMode::SaveFile, title, dir, filter, !confirmOverwrite ? QFileDialog::DontConfirmOverwrite : 0);

    RetVal<Val> rv = openSync(q);
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

    RetVal<Val> rv = openSync(q);
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

static std::vector<QColor> getCustomColors()
{
    const int customColorCount = QColorDialog::customCount();
    std::vector<QColor> customColors;
    customColors.reserve(customColorCount);
    for (int i = 0; i < customColorCount; ++i) {
        customColors.push_back(QColorDialog::customColor(i));
    }

    return customColors;
}

static void setCustomColors(const std::vector<QColor>& customColors)
{
    const int customColorCount = std::min(QColorDialog::customCount(), static_cast<int>(customColors.size()));
    for (int i = 0; i < customColorCount; ++i) {
        QColorDialog::setCustomColor(i, customColors[i]);
    }
}

async::Promise<Color> Interactive::selectColor(const Color& color, const std::string& title, bool allowAlpha)
{
    if (m_isSelectColorOpened) {
        LOGW() << "already opened";
        return async::make_promise<Color>([](auto, auto reject) {
            Ret ret = muse::make_ret(Ret::Code::UnknownError);
            return reject(ret.code(), ret.text());
        });
    }

    m_isSelectColorOpened = true;

    setCustomColors(uiConfiguration()->colorDialogCustomColors());

    return async::make_promise<Color>([this, color, title, allowAlpha](auto resolve, auto reject) {
        //! FIX https://github.com/musescore/MuseScore/issues/23208
        shortcutsRegister()->setActive(false);

        QColorDialog* dlg = new QColorDialog();
        if (!title.empty()) {
            dlg->setWindowTitle(QString::fromStdString(title));
        }

        QColor currentColor = color.toQColor();

        // If the color is fully transparent, set alpha to opaque, to avoid "Hm, nothing happened" user confusion
        if (currentColor.alpha() == 0) {
            currentColor.setAlpha(255);
        }

        dlg->setCurrentColor(currentColor);
        dlg->setOption(QColorDialog::ShowAlphaChannel, allowAlpha);

        QObject::connect(dlg, &QColorDialog::finished, [this, dlg, resolve, reject](int result) {
            dlg->deleteLater();

            uiConfiguration()->setColorDialogCustomColors(getCustomColors());

            m_isSelectColorOpened = false;
            shortcutsRegister()->setActive(true);

            if (result != QDialog::Accepted) {
                Ret ret = muse::make_ret(Ret::Code::Cancel);
                (void)reject(ret.code(), ret.text());
                return;
            }

            QColor selectedColor = dlg->selectedColor();
            (void)resolve(Color::fromQColor(selectedColor));
        });

        dlg->open();

        return async::Promise<Color>::dummy_result();
    }, async::PromiseType::AsyncByBody);
}

bool Interactive::isSelectColorOpened() const
{
    return m_isSelectColorOpened;
}

RetVal<Val> Interactive::openSync(const UriQuery& q)
{
#ifndef MUSE_MODULE_INTERACTIVE_SYNC_SUPPORTED
    NOT_SUPPORTED;
    std::abort();
    {
        RetVal<Val> rv;
        rv.ret = muse::make_ret(Ret::Code::NotSupported);
        return rv;
    }
#endif

    RetVal<Val> rv;
    QEventLoop loop;
    Promise<Val>::Resolve resolve;
    Promise<Val>::Reject reject;
    Promise<Val> promise = async::make_promise<Val>([&resolve, &reject](auto res, auto rej) {
        resolve = res;
        reject = rej;
        return Promise<Val>::Result::unchecked();
    }, PromiseType::AsyncByBody);

    promise.onResolve(this, [&rv, &loop](const Val& val) {
        rv = RetVal<Val>::make_ok(val);
        loop.quit();
    });

    promise.onReject(this, [&rv, &loop](int code, const std::string& err) {
        LOGE() << code << " " << err;
        rv.ret = make_ret(code, err);
        loop.quit();
    });

    auto func = openFunc(q);
    func(resolve, reject);

    ContainerMeta openMeta = uriRegister()->meta(q.uri());
    if (openMeta.type == ContainerMeta::PrimaryPage) {
        LOGW() << "Primary pages should not open in synchronous mode, please fix this.";
        return rv;
    }

    loop.exec();

    return rv;
}

async::Promise<Val> Interactive::open(const UriQuery& uri)
{
    return openAsync(uri);
}

Promise<Val> Interactive::openAsync(const UriQuery& q)
{
    return async::make_promise<Val>(openFunc(q), PromiseType::AsyncByBody);
}

async::Promise<Val> Interactive::openAsync(const Uri& uri, const QVariantMap& params)
{
    return async::make_promise<Val>(openFunc(UriQuery(uri), params), PromiseType::AsyncByBody);
}

Promise<Val>::BodyResolveReject Interactive::openFunc(const UriQuery& q)
{
    QVariantMap params;
    const UriQuery::Params& p = q.params();
    for (auto it = p.cbegin(); it != p.cend(); ++it) {
        params[QString::fromStdString(it->first)] = it->second.toQVariant();
    }

    return openFunc(q, params);
}

Promise<Val>::BodyResolveReject Interactive::openFunc(const UriQuery& q, const QVariantMap& params)
{
    auto func = [this, q, params](Promise<Val>::Resolve resolve, Promise<Val>::Reject reject) {
        IF_ASSERT_FAILED(!m_openingObject.objectId.isValid()) {
            LOGE() << "The opening of the previous object has not been completed"
                   << ", objectId: " << m_openingObject.objectId.toString()
                   << ", query: " << m_openingObject.query.toString();
        }

        m_openingObject = { q, resolve, reject, QVariant(), nullptr };

        RetVal<OpenData> openedRet;

        notifyAboutCurrentUriWillBeChanged();

        //! NOTE Currently, extensions do not replace the default functionality
        //! But in the future, we may allow extensions to replace the current functionality
        //! (first check for the presence of an extension with this uri,
        //! and if it is found, then open it)

        ContainerMeta openMeta = uriRegister()->meta(q.uri());
        switch (openMeta.type) {
        case ContainerMeta::QWidgetDialog:
            openedRet = openWidgetDialog(q.uri(), params);
            break;
        case ContainerMeta::PrimaryPage:
        case ContainerMeta::QmlDialog:
            openedRet = openQml(q.uri(), params);
            break;
        case ContainerMeta::Undefined: {
            //! NOTE Not found default, try extension
            extensions::Manifest ext = extensionsProvider()->manifest(q.uri());
            if (ext.isValid()) {
                openedRet = openExtensionDialog(q, params);
            } else {
                openedRet.ret = make_ret(Ret::Code::UnknownError);
            }
        }
        }

        if (!openedRet.ret) {
            LOGE() << "failed open err: " << openedRet.ret.toString() << ", uri: " << q.toString();
            return reject(openedRet.ret.code(), openedRet.ret.text());
        }

        return Promise<Val>::Result::unchecked();
    };

    return func;
}

RetVal<bool> Interactive::isOpened(const UriQuery& uri) const
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        if (objectInfo.query == uri) {
            return RetVal<bool>::make_ok(true);
        }
    }

    return RetVal<bool>::make_ok(false);
}

RetVal<bool> Interactive::isOpened(const Uri& uri) const
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        if (objectInfo.query.uri() == uri) {
            return RetVal<bool>::make_ok(true);
        }
    }

    return RetVal<bool>::make_ok(false);
}

RetVal<bool> Interactive::isOpened(const QString& objectId) const
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        if (objectInfo.objectId == objectId) {
            return RetVal<bool>::make_ok(true);
        }
    }

    return RetVal<bool>::make_ok(false);
}

async::Channel<Uri> Interactive::opened() const
{
    return m_opened;
}

void Interactive::raise(const UriQuery& uri)
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        if (objectInfo.query != uri) {
            continue;
        }

        ContainerMeta openMeta = uriRegister()->meta(objectInfo.query.uri());
        switch (openMeta.type) {
        case ContainerMeta::QWidgetDialog: {
            if (auto window = dynamic_cast<QWidget*>(objectInfo.window)) {
                window->raise();
                window->activateWindow();
            }
        } break;
        case ContainerMeta::QmlDialog:
            raiseQml(objectInfo.objectId);
            break;
        case ContainerMeta::PrimaryPage:
        case ContainerMeta::Undefined:
            break;
        }
    }
}

void Interactive::close(const UriQuery& uri)
{
    for (const ObjectInfo& obj : allOpenObjects()) {
        if (obj.query == uri) {
            closeObject(obj);
        }
    }
}

void Interactive::close(const Uri& uri)
{
    for (const ObjectInfo& obj : allOpenObjects()) {
        if (obj.query.uri() == uri) {
            closeObject(obj);
        }
    }
}

void Interactive::closeAllDialogs()
{
    for (const ObjectInfo& objectInfo: allOpenObjects()) {
        UriQuery uriQuery = objectInfo.query;
        if (muse::diagnostics::isDiagnosticsUri(uriQuery.uri())) {
            continue;
        }
        ContainerMeta openMeta = uriRegister()->meta(uriQuery.uri());
        if (openMeta.type == ContainerMeta::QWidgetDialog || openMeta.type == ContainerMeta::QmlDialog) {
            closeObject(objectInfo);
        }
    }
}

void Interactive::closeObject(const ObjectInfo& obj)
{
    ContainerMeta openMeta = uriRegister()->meta(obj.query.uri());
    switch (openMeta.type) {
    case ContainerMeta::QWidgetDialog: {
        if (auto window = dynamic_cast<QWidget*>(obj.window)) {
            window->close();
        }
    } break;
    case ContainerMeta::QmlDialog:
        closeQml(obj.objectId);
        break;
    case ContainerMeta::PrimaryPage:
    case ContainerMeta::Undefined:
        break;
    }
}

void Interactive::fillExtData(QmlLaunchData* data, const UriQuery& q, const QVariantMap& params_) const
{
    static Uri VIEWER_URI = Uri("muse://extensions/viewer");

    ContainerMeta meta = uriRegister()->meta(VIEWER_URI);
    data->setValue("module", meta.qmlModule);
    data->setValue("path", meta.qmlPath);
    data->setValue("type", meta.type);

    QVariantMap params = params_;
    params["uri"] = QString::fromStdString(q.toString());

    //! NOTE Extension dialogs open as non-modal by default
    //! The modal parameter must be present in the uri
    //! But here, just in case, `true` is indicated by default,
    //! since this value is set in the base class of the dialog by default
    if (!params.contains("modal")) {
        params["modal"] = q.param("modal", Val(true)).toBool();
    }

    data->setValue("uri", QString::fromStdString(VIEWER_URI.toString()));
    data->setValue("params", params);
}

void Interactive::fillData(QmlLaunchData* data, const Uri& uri, const QVariantMap& params) const
{
    ContainerMeta meta = uriRegister()->meta(uri);
    data->setValue("module", meta.qmlModule);
    data->setValue("path", meta.qmlPath);
    data->setValue("type", meta.type);
    data->setValue("uri", QString::fromStdString(uri.toString()));
    data->setValue("params", params);
    data->setValue("modal", params.value("modal", ""));
}

void Interactive::fillData(QObject* object, const QVariantMap& params) const
{
    const QMetaObject* metaObject = object->metaObject();
    for (int i = 0; i < metaObject->propertyCount(); i++) {
        QMetaProperty metaProperty = metaObject->property(i);
        if (params.contains(metaProperty.name())) {
            object->setProperty(metaProperty.name(), params[metaProperty.name()]);
        }
    }
}

ValCh<Uri> Interactive::currentUri() const
{
    ValCh<Uri> v;
    if (!m_stack.empty()) {
        v.val = m_stack.last().query.uri();
    }
    v.ch = m_currentUriChanged;
    return v;
}

RetVal<bool> Interactive::isCurrentUriDialog() const
{
    if (m_stack.empty()) {
        return RetVal<bool>::make_ok(false);
    }

    const ObjectInfo& last = m_stack.last();
    if (!last.window) {
        return RetVal<bool>::make_ok(false);
    }

    return RetVal<bool>::make_ok(last.window != mainWindow()->qWindow());
}

async::Notification Interactive::currentUriAboutToBeChanged() const
{
    return m_currentUriAboutToBeChanged;
}

std::vector<Uri> Interactive::stack() const
{
    std::vector<Uri> uris;
    for (const ObjectInfo& info : m_stack) {
        uris.push_back(info.query.uri());
    }
    return uris;
}

QWindow* Interactive::topWindow() const
{
    QWindow* mainWin = mainWindow()->qWindow();
    if (!mainWin) {
        mainWin = qApp->focusWindow();
    }

    if (m_stack.empty()) {
        LOGE() << "stack is empty";
        return mainWin;
    }

    const ObjectInfo& last = m_stack.last();
    if (!last.window) {
        return mainWin;
    }

    if (last.window == mainWin) {
        return mainWin;
    }

    // TODO/HACK: last.window doesn't seem to have a parent when the top window is a widget....
    if (!last.window->parent() && !topWindowIsWidget()) {
        ASSERT_X("Window must have a parent!");
    }

    if (!last.window->isWidgetType()) {
        QWindow* qwindow = qobject_cast<QWindow*>(last.window);
        IF_ASSERT_FAILED(qwindow) {
            return mainWin;
        }
        return qwindow;
    }

    // QWidget
    QWidget* qwidget = qobject_cast<QWidget*>(last.window);
    QWindow* qwindow = qwidget->windowHandle();
    IF_ASSERT_FAILED(qwindow) {
        return mainWin;
    }
    return qwindow;
}

bool Interactive::topWindowIsWidget() const
{
    if (m_stack.empty()) {
        return false;
    }

    const ObjectInfo& last = m_stack.last();
    if (!last.window) {
        return false;
    }

    return last.window->isWidgetType();
}

QString Interactive::objectId(const QVariant& val) const
{
    static int count(0);

    ++count;

    QString objectId;
    if (val.canConvert<QObject*>()) {
        QObject* obj = val.value<QObject*>();
        IF_ASSERT_FAILED(obj) {
            return QString();
        }

        objectId = QString(obj->metaObject()->className()) + "_" + QString::number(count);
    } else {
        objectId = "unknown_" + QString::number(count);
    }
    return "object://" + objectId;
}

Ret Interactive::toRet(const QVariant& jsr) const
{
    QVariantMap jsobj = jsr.toMap();
    IF_ASSERT_FAILED(jsobj.contains("errcode")) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ret ret;
    ret.setCode(jsobj.value("errcode").toInt());
    return ret;
}

RetVal<Val> Interactive::toRetVal(const QVariant& jsrv) const
{
    RetVal<Val> rv;
    QVariantMap jsobj = jsrv.toMap();

    IF_ASSERT_FAILED(jsobj.contains("errcode")) {
        rv.ret = make_ret(Ret::Code::UnknownError);
        return rv;
    }

    int errcode = jsobj.value("errcode").toInt();
    QVariant val = jsobj.value("value");

    rv.ret.setCode(errcode);
    rv.val = Val::fromQVariant(val);

    return rv;
}

RetVal<Interactive::OpenData> Interactive::openExtensionDialog(const UriQuery& q, const QVariantMap& params)
{
    QmlLaunchData data;
    fillExtData(&data, q, params);

    m_openRequested.send(&data);

    RetVal<OpenData> result;
    result.ret = toRet(data.value("ret"));
    result.val.objectId = data.value("objectId").toString();

    return result;
}

RetVal<Interactive::OpenData> Interactive::openWidgetDialog(const Uri& uri, const QVariantMap& params)
{
    RetVal<OpenData> result;

    ContainerMeta meta = uriRegister()->meta(uri);
    int widgetMetaTypeId = meta.widgetMetaTypeId;

    static int count(0);
    QString objectId = QString("%1_%2").arg(widgetMetaTypeId).arg(++count);

    QMetaType metaType = QMetaType(widgetMetaTypeId);
    QDialog* dialog = static_cast<QDialog*>(metaType.create());

    if (!dialog) {
        result.ret = make_ret(Ret::Code::UnknownError);
        return result;
    }

    fillData(dialog, params);

    //! NOTE Will be deleted with the dialog
    WidgetDialogAdapter* adapter = new WidgetDialogAdapter(dialog);
    adapter->onShow([this, objectId, dialog]() {
        if (isOpened(objectId).val) {
            LOGE() << "Dialog is already opened: " << objectId << ", ignoring this show event";
            return;
        }

        async::Async::call(this, [this, objectId, dialog]() {
            onOpen(ContainerMeta::QWidgetDialog, objectId, dialog->window());
        });
    })
    .onHide([this, objectId, dialog]() {
        if (!isOpened(objectId).val) {
            LOGE() << "Dialog is not opened: " << objectId << ", ignoring this hide event";
            return;
        }

        QDialog::DialogCode dialogCode = static_cast<QDialog::DialogCode>(dialog->result());
        Ret::Code errorCode = dialogCode == QDialog::Accepted ? Ret::Code::Ok : Ret::Code::Cancel;

        QVariantMap ret;
        ret["errcode"] = static_cast<int>(errorCode);

        onClose(objectId, ret);

        dialog->deleteLater();
    });

    bool modal = params.value("modal", "true") == "true";
    dialog->setWindowModality(modal ? Qt::ApplicationModal : Qt::NonModal);
    dialog->show();
    dialog->activateWindow(); // give keyboard focus to aid blind users

    result.ret = make_ret(Ret::Code::Ok);
    result.val.objectId = objectId;

    return result;
}

RetVal<Interactive::OpenData> Interactive::openQml(const Uri& uri, const QVariantMap& params)
{
    QmlLaunchData data;
    fillData(&data, uri, params);

    m_openRequested.send(&data);

    RetVal<OpenData> result;
    result.ret = toRet(data.value("ret"));
    result.val.objectId = data.value("objectId").toString();

    return result;
}

void Interactive::closeQml(const QVariant& objectId)
{
    m_closeRequested.send(objectId);
}

void Interactive::raiseQml(const QVariant& objectId)
{
    m_raiseRequested.send(objectId);
}

void Interactive::onOpen(const QVariant& type, const QVariant& objectId, QObject* window)
{
    ContainerMeta::Type containerMeta = type.value<ContainerMeta::Type>();

    IF_ASSERT_FAILED(containerMeta != ContainerMeta::Undefined) {
        containerMeta = ContainerMeta::QmlDialog;
    }

    m_openingObject.objectId = objectId;
    m_openingObject.window = window;
    if (!m_openingObject.window) {
        m_openingObject.window = (containerMeta == ContainerMeta::PrimaryPage) ? mainWindow()->qWindow() : qApp->focusWindow();
    }

    if (m_openingObject.query.param("floating").toBool()) {
        m_floatingObjects.push_back(m_openingObject);
        m_openingObject = ObjectInfo(); // clear
        return;
    }

    if (ContainerMeta::PrimaryPage == containerMeta) {
        // Replace bottom item of the stack, because that always reflects the current PrimaryPage
        if (m_stack.empty()) {
            m_stack.push(m_openingObject);
        } else {
            m_stack[0] = m_openingObject;
        }
    } else if (ContainerMeta::QmlDialog == containerMeta) {
        m_stack.push(m_openingObject);
    } else if (ContainerMeta::QWidgetDialog == containerMeta) {
        m_stack.push(m_openingObject);
    } else {
        IF_ASSERT_FAILED_X(false, "unknown page type") {
            m_stack.push(m_openingObject);
        }
    }

    notifyAboutCurrentUriChanged();

    Uri uri = m_openingObject.query.uri();
    m_openingObject = ObjectInfo(); // clear

    Async::call(this, [this, uri]() {
        m_opened.send(uri);
    });
}

void Interactive::onClose(const QString& objectId, const QVariant& jsrv)
{
    RetVal<Val> rv = toRetVal(jsrv);

    ObjectInfo obj;

    bool inStack = false;
    for (int i = 0; i < m_stack.size(); ++i) {
        if (m_stack.at(i).objectId == objectId) {
            obj = m_stack.at(i);
            inStack = true;
            m_stack.remove(i);
            break;
        }
    }

    if (!inStack) {
        for (size_t i = 0; i < m_floatingObjects.size(); ++i) {
            if (m_floatingObjects.at(i).objectId == objectId) {
                obj = m_floatingObjects.at(i);
                m_floatingObjects.erase(m_floatingObjects.begin() + i);
                break;
            }
        }
    }

    DO_ASSERT(obj.objectId.isValid());

    if (rv.ret) {
        (void)obj.resolve(rv.val);
    } else {
        (void)obj.reject(rv.ret.code(), rv.ret.text());
    }

    if (inStack) {
        notifyAboutCurrentUriChanged();
    }
}

std::vector<Interactive::ObjectInfo> Interactive::allOpenObjects() const
{
    std::vector<ObjectInfo> result;

    result.insert(result.end(), m_stack.cbegin(), m_stack.cend());
    result.insert(result.end(), m_floatingObjects.cbegin(), m_floatingObjects.cend());

    return result;
}

void Interactive::notifyAboutCurrentUriChanged()
{
    m_currentUriChanged.send(currentUri().val);
}

void Interactive::notifyAboutCurrentUriWillBeChanged()
{
    m_currentUriAboutToBeChanged.notify();
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

Ret Interactive::canOpenApp(const UriQuery& uri) const
{
#ifdef Q_OS_MACOS
    return MacOSInteractiveHelper::canOpenApp(uri);
#else
    NOT_IMPLEMENTED;
    UNUSED(uri);
    return false;
#endif
}

async::Promise<Ret> Interactive::openApp(const UriQuery& uri) const
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
