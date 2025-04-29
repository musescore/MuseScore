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
#ifndef MUSE_UI_INTERACTIVEPROVIDER_H
#define MUSE_UI_INTERACTIVEPROVIDER_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QStack>
#include <QEventLoop>

#include "modularity/ioc.h"
#include "../iinteractiveprovider.h"
#include "../iinteractiveuriregister.h"
#include "../imainwindow.h"
#include "extensions/iextensionsprovider.h"
#include "types/retval.h"

namespace muse::ui {
class QmlLaunchData : public QObject
{
    Q_OBJECT
public:
    explicit QmlLaunchData(QObject* parent = nullptr);

    Q_INVOKABLE QVariant value(const QString& key) const;
    Q_INVOKABLE void setValue(const QString& key, const QVariant& val);
    Q_INVOKABLE QVariant data() const;

private:
    QVariantMap m_data;
};

class InteractiveProvider : public QObject, public IInteractiveProvider, public Injectable
{
    Q_OBJECT

    Inject<IInteractiveUriRegister> uriRegister = { this };
    Inject<IMainWindow> mainWindow = { this };
    Inject<muse::extensions::IExtensionsProvider> extensionsProvider = { this };

public:
    explicit InteractiveProvider(const modularity::ContextPtr& iocCtx);

    RetVal<Val> question(const std::string& contentTitle, const IInteractive::Text& text, const IInteractive::ButtonDatas& buttons,
                         int defBtn = int(IInteractive::Button::NoButton), const IInteractive::Options& options = {},
                         const std::string& dialogTitle = "") override;

    RetVal<Val> info(const std::string& contentTitle, const IInteractive::Text& text, const IInteractive::ButtonDatas& buttons,
                     int defBtn = int(IInteractive::Button::NoButton), const IInteractive::Options& options = {},
                     const std::string& dialogTitle = "") override;

    RetVal<Val> warning(const std::string& contentTitle, const IInteractive::Text& text, const std::string& detailedText = {},
                        const IInteractive::ButtonDatas& buttons = {}, int defBtn = int(IInteractive::Button::NoButton),
                        const IInteractive::Options& options = {}, const std::string& dialogTitle = "") override;

    RetVal<Val> error(const std::string& contentTitle, const IInteractive::Text& text, const std::string& detailedText = {},
                      const IInteractive::ButtonDatas& buttons = {}, int defBtn = int(IInteractive::Button::NoButton),
                      const IInteractive::Options& options = {}, const std::string& dialogTitle = "") override;

    Ret showProgress(const std::string& title, Progress* progress) override;

    RetVal<io::path_t> selectOpeningFile(const std::string& title, const io::path_t& dir, const std::vector<std::string>& filter) override;
    RetVal<io::path_t> selectSavingFile(const std::string& title, const io::path_t& path, const std::vector<std::string>& filter,
                                        bool confirmOverwrite) override;
    RetVal<io::path_t> selectDirectory(const std::string& title, const io::path_t& dir) override;

    RetVal<QColor> selectColor(const QColor& color = Qt::white, const QString& title = "") override;
    bool isSelectColorOpened() const override;

    RetVal<Val> open(const UriQuery& uri) override;
    RetVal<bool> isOpened(const Uri& uri) const override;
    RetVal<bool> isOpened(const UriQuery& uri) const override;
    async::Channel<Uri> opened() const override;

    void raise(const UriQuery& uri) override;

    void close(const Uri& uri) override;
    void close(const UriQuery& uri) override;
    void closeAllDialogs() override;

    ValCh<Uri> currentUri() const override;
    RetVal<bool> isCurrentUriDialog() const override;
    async::Notification currentUriAboutToBeChanged() const override;
    std::vector<Uri> stack() const override;

    Q_INVOKABLE QWindow* topWindow() const override;
    bool topWindowIsWidget() const override;

    Q_INVOKABLE QString objectId(const QVariant& val) const;

    Q_INVOKABLE void onOpen(const QVariant& type, const QVariant& objectId, QObject* window = nullptr);
    Q_INVOKABLE void onClose(const QString& objectId, const QVariant& rv);

signals:
    void fireOpen(muse::ui::QmlLaunchData* data);
    void fireClose(QVariant data);
    void fireRaise(QVariant data);

    void fireOpenStandardDialog(muse::ui::QmlLaunchData* data);
    void fireOpenFileDialog(muse::ui::QmlLaunchData* data);
    void fireOpenProgressDialog(muse::ui::QmlLaunchData* data);

private:
    struct OpenData
    {
        bool sync = false;
        QString objectId;
    };

    struct ObjectInfo
    {
        UriQuery uriQuery;
        QVariant objectId;
        QObject* window = nullptr;
    };

    enum class FileDialogType {
        SelectOpenningFile,
        SelectSavingFile,
        SelectDirectory
    };

    void raiseWindowInStack(QObject* newActiveWindow);

    void fillExtData(QmlLaunchData* data, const UriQuery& q) const;
    void fillData(QmlLaunchData* data, const UriQuery& q) const;
    void fillData(QObject* object, const UriQuery& q) const;
    void fillStandardDialogData(QmlLaunchData* data, const QString& type, const std::string& contentTitle, const IInteractive::Text& text,
                                const std::string& detailedText, const IInteractive::ButtonDatas& buttons, int defBtn,
                                const IInteractive::Options& options, const std::string& dialogTitle) const;
    void fillFileDialogData(QmlLaunchData* data, FileDialogType type, const std::string& title, const io::path_t& path,
                            const std::vector<std::string>& filter = {}, bool confirmOverwrite = true) const;

    Ret toRet(const QVariant& jsr) const;
    RetVal<Val> toRetVal(const QVariant& jsrv) const;

    RetVal<OpenData> openExtensionDialog(const UriQuery& q);
    RetVal<OpenData> openWidgetDialog(const UriQuery& q);
    RetVal<OpenData> openQml(const UriQuery& q);
    RetVal<Val> openStandardDialog(const QString& type, const std::string& contentTitle, const IInteractive::Text& text,
                                   const std::string& detailedText = {}, const IInteractive::ButtonDatas& buttons = {},
                                   int defBtn = int(IInteractive::Button::NoButton), const IInteractive::Options& options = {},
                                   const std::string& dialogTitle = "");

    RetVal<io::path_t> openFileDialog(FileDialogType type, const std::string& title, const io::path_t& path,
                                      const std::vector<std::string>& filter = {}, bool confirmOverwrite = true);

    void closeObject(const ObjectInfo& obj);

    void closeQml(const QVariant& objectId);
    void raiseQml(const QVariant& objectId);

    std::vector<ObjectInfo> allOpenObjects() const;

    void notifyAboutCurrentUriChanged();
    void notifyAboutCurrentUriWillBeChanged();

    UriQuery m_openingUriQuery;

    QStack<ObjectInfo> m_stack;
    std::vector<ObjectInfo> m_floatingObjects;

    async::Channel<Uri> m_currentUriChanged;
    async::Notification m_currentUriAboutToBeChanged;
    QMap<QString, RetVal<Val> > m_retvals;
    async::Channel<Uri> m_opened;

    QEventLoop m_fileDialogEventLoop;

    bool m_isSelectColorOpened = false;
};
}

#endif // MUSE_UI_INTERACTIVEPROVIDER_H
