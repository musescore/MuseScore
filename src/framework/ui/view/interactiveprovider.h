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

#include "global/async/asyncable.h"

#include "modularity/ioc.h"
#include "../iinteractiveprovider.h"
#include "../iinteractiveuriregister.h"
#include "../imainwindow.h"
#include "extensions/iextensionsprovider.h"
#include "shortcuts/ishortcutsregister.h"
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

class InteractiveProvider : public QObject, public IInteractiveProvider, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Inject<IInteractiveUriRegister> uriRegister = { this };
    Inject<IMainWindow> mainWindow = { this };
    Inject<muse::extensions::IExtensionsProvider> extensionsProvider = { this };
    Inject<shortcuts::IShortcutsRegister> shortcutsRegister = { this };

public:
    explicit InteractiveProvider(const modularity::ContextPtr& iocCtx);

    async::Promise<Color> selectColor(const Color& color = Color::WHITE, const std::string& title = "") override;
    bool isSelectColorOpened() const override;

    RetVal<Val> openSync(const UriQuery& uri) override;
    async::Promise<Val> openAsync(const UriQuery& uri) override;
    async::Promise<Val> openAsync(const Uri& uri, const QVariantMap& params) override;
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

    ObjectInfo m_openingObject;

    QStack<ObjectInfo> m_stack;
    std::vector<ObjectInfo> m_floatingObjects;

    async::Channel<Uri> m_currentUriChanged;
    async::Notification m_currentUriAboutToBeChanged;
    async::Channel<Uri> m_opened;

    bool m_isSelectColorOpened = false;
};
}

#endif // MUSE_UI_INTERACTIVEPROVIDER_H
