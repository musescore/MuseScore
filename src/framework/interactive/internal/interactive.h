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

#pragma once

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QStack>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "extensions/iextensionsprovider.h"
#include "shortcuts/ishortcutsregister.h"
#include "ui/imainwindow.h"
#include "ui/iuiconfiguration.h"

#include "../iinteractive.h"
#include "iinteractiveprovider.h"
#include "iinteractiveuriregister.h"

namespace muse::interactive {
class Interactive : public QObject, public IInteractive, public IInteractiveProvider, public Contextable, public async::Asyncable
{
    Q_OBJECT

    GlobalInject<ui::IUiConfiguration> uiConfiguration;
    ContextInject<interactive::IInteractiveUriRegister> uriRegister = { this };
    ContextInject<extensions::IExtensionsProvider> extensionsProvider = { this };
    ContextInject<shortcuts::IShortcutsRegister> shortcutsRegister = { this };
    ContextInject<ui::IMainWindow> mainWindow = { this };

public:
    explicit Interactive(const muse::modularity::ContextPtr& ctx);

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
    void showProgress(const std::string& title, Progress progress) override;

    // files
    async::Promise<io::path_t> selectOpeningFile(const std::string& title, const io::path_t& dir,
                                                 const std::vector<std::string>& filter) override;
    io::path_t selectOpeningFileSync(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter,
                                     const int options) override;
    io::path_t selectSavingFileSync(const std::string& title, const io::path_t& path, const std::vector<std::string>& filter,
                                    bool confirmOverwrite = true) override;

    // dirs
    io::path_t selectDirectory(const std::string& title, const io::path_t& dir) override;
    io::paths_t selectMultipleDirectories(const std::string& title, const io::path_t& dir, const io::paths_t& selectedDirectories) override;

    // color
    async::Promise<Color> selectColor(const Color& color = Color::WHITE, const std::string& title = {}, bool allowAlpha = false) override;
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

    // state
    ValCh<Uri> currentUri() const override;
    RetVal<bool> isCurrentUriDialog() const override;
    async::Notification currentUriAboutToBeChanged() const override;
    std::vector<Uri> stack() const override;

    QWindow* topWindow() const override;
    bool topWindowIsWidget() const override;

    // external
    Ret openUrl(const std::string& url) const override;
    Ret openUrl(const QUrl& url) const override;

    Ret isAppExists(const std::string& appIdentifier) const override;
    Ret canOpenApp(const UriQuery& uri) const override;
    async::Promise<Ret> openApp(const UriQuery& uri) const override;

    Ret revealInFileBrowser(const io::path_t& filePath) const override;

    // IInteractiveProvider interface
    QString objectId(const QVariant& val) const override;

    void onOpen(const QVariant& type, const QVariant& objectId, QObject* window = nullptr) override;
    void onClose(const QString& objectId, const QVariant& rv) override;

    async::Channel<QmlLaunchData*> openRequested() const override { return m_openRequested; }
    async::Channel<QVariant> closeRequested() const override { return m_closeRequested; }
    async::Channel<QVariant> raiseRequested() const override { return m_raiseRequested; }

private:
    struct OpenData {
        QString objectId;
    };

    struct ObjectInfo {
        UriQuery query;
        async::Promise<Val>::Resolve resolve;
        async::Promise<Val>::Reject reject;
        QVariant objectId;
        QObject* window = nullptr;
    };

    async::Promise<Val> openAsync(const UriQuery& uri);
    async::Promise<Val> openAsync(const Uri& uri, const QVariantMap& params);

    async::Promise<Val>::BodyResolveReject openFunc(const UriQuery& q);
    async::Promise<Val>::BodyResolveReject openFunc(const UriQuery& q, const QVariantMap& params);

    void raiseWindowInStack(QObject* newActiveWindow);

    void fillExtData(QmlLaunchData* data, const UriQuery& q, const QVariantMap& params) const;
    void fillData(QmlLaunchData* data, const Uri& uri, const QVariantMap& params) const;
    void fillData(QObject* object, const QVariantMap& params) const;

    Ret toRet(const QVariant& jsr) const;
    RetVal<Val> toRetVal(const QVariant& jsrv) const;

    RetVal<OpenData> openExtensionDialog(const UriQuery& q, const QVariantMap& params);
    RetVal<OpenData> openWidgetDialog(const Uri& uri, const QVariantMap& params);
    RetVal<OpenData> openQml(const Uri& uri, const QVariantMap& params);

    void closeObject(const ObjectInfo& obj);

    void closeQml(const QVariant& objectId);
    void raiseQml(const QVariant& objectId);

    std::vector<ObjectInfo> allOpenObjects() const;

    void notifyAboutCurrentUriChanged();
    void notifyAboutCurrentUriWillBeChanged();

    UriQuery makeQuery(const std::string& type, const std::string& contentTitle, const Text& text, const ButtonDatas& buttons, int defBtn,
                       const Options& options, const std::string& dialogTitle) const;

    IInteractive::Result makeResult(const Val& val) const;

    async::Promise<IInteractive::Result> openStandardAsync(const std::string& type, const std::string& contentTitle, const Text& text,
                                                           const ButtonDatas& buttons, int defBtn, const Options& options,
                                                           const std::string& dialogTitle);

    IInteractive::Result openStandardSync(const std::string& type, const std::string& contentTitle, const Text& text,
                                          const ButtonDatas& buttons, int defBtn, const Options& options, const std::string& dialogTitle);

    ObjectInfo m_openingObject;

    QStack<ObjectInfo> m_stack;
    std::vector<ObjectInfo> m_floatingObjects;

    async::Channel<Uri> m_currentUriChanged;
    async::Notification m_currentUriAboutToBeChanged;
    async::Channel<Uri> m_opened;

    async::Channel<QmlLaunchData*> m_openRequested;
    async::Channel<QVariant> m_closeRequested;
    async::Channel<QVariant> m_raiseRequested;

    bool m_isSelectColorOpened = false;
};
}
