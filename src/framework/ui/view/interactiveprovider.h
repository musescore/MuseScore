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
#ifndef MU_UI_INTERACTIVEPROVIDER_H
#define MU_UI_INTERACTIVEPROVIDER_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QStack>

#include "modularity/ioc.h"
#include "../iinteractiveprovider.h"
#include "../iinteractiveuriregister.h"
#include "../imainwindow.h"
#include "retval.h"

namespace mu::ui {
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

class InteractiveProvider : public QObject, public IInteractiveProvider
{
    Q_OBJECT

    INJECT(ui, IInteractiveUriRegister, uriRegister)
    INJECT(ui, IMainWindow, mainWindow)

public:
    explicit InteractiveProvider();

    RetVal<Val> question(const std::string& title, const framework::IInteractive::Text& text,
                         const framework::IInteractive::ButtonDatas& buttons, int defBtn = int(framework::IInteractive::Button::NoButton),
                         const framework::IInteractive::Options& options = {}) override;

    RetVal<Val> info(const std::string& title, const std::string& text, const framework::IInteractive::ButtonDatas& buttons,
                     int defBtn = int(framework::IInteractive::Button::NoButton),
                     const framework::IInteractive::Options& options = {}) override;
    RetVal<Val> warning(const std::string& title, const std::string& text, const framework::IInteractive::ButtonDatas& buttons,
                        int defBtn = int(framework::IInteractive::Button::NoButton),
                        const framework::IInteractive::Options& options = {}) override;
    RetVal<Val> error(const std::string& title, const std::string& text, const framework::IInteractive::ButtonDatas& buttons,
                      int defBtn = int(framework::IInteractive::Button::NoButton),
                      const framework::IInteractive::Options& options = {}) override;

    RetVal<Val> open(const UriQuery& uri) override;
    RetVal<bool> isOpened(const Uri& uri) const override;
    RetVal<bool> isOpened(const UriQuery& uri) const override;
    async::Channel<Uri> opened() const override;

    void raise(const UriQuery& uri) override;

    void close(const Uri& uri) override;

    ValCh<Uri> currentUri() const override;
    std::vector<Uri> stack() const override;

    QWindow* topWindow() const override;
    bool topWindowIsWidget() const override;

    Q_INVOKABLE QString objectId(const QVariant& val) const;

    Q_INVOKABLE void onOpen(const QVariant& type, const QVariant& objectId, QObject* window = nullptr);
    Q_INVOKABLE void onClose(const QString& objectId, const QVariant& rv);

signals:
    void fireOpen(QmlLaunchData* data);
    void fireClose(QVariant data);
    void fireRaise(QVariant data);

    void fireOpenStandardDialog(QmlLaunchData* data);

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

    void raiseWindowInStack(QObject* newActiveWindow);

    void fillData(QmlLaunchData* data, const UriQuery& q) const;
    void fillData(QObject* object, const UriQuery& q) const;
    void fillStandardDialogData(QmlLaunchData* data, const QString& type, const QString& title, const framework::IInteractive::Text& text,
                                const framework::IInteractive::ButtonDatas& buttons, int defBtn,
                                const framework::IInteractive::Options& options) const;

    Ret toRet(const QVariant& jsr) const;
    RetVal<Val> toRetVal(const QVariant& jsrv) const;

    RetVal<OpenData> openWidgetDialog(const UriQuery& q);
    RetVal<OpenData> openQml(const UriQuery& q);
    RetVal<Val> openStandardDialog(const QString& type, const QString& title, const framework::IInteractive::Text& text,
                                   const framework::IInteractive::ButtonDatas& buttons,
                                   int defBtn = int(framework::IInteractive::Button::NoButton),
                                   const framework::IInteractive::Options& options = {});

    void closeQml(const QVariant& objectId);
    void raiseQml(const QVariant& objectId);

    void notifyAboutCurrentUriChanged();

    UriQuery m_openingUriQuery;
    QStack<ObjectInfo> m_stack;
    async::Channel<Uri> m_currentUriChanged;
    QMap<QString, RetVal<Val> > m_retvals;
    async::Channel<Uri> m_opened;
};
}

#endif // MU_UI_INTERACTIVEPROVIDER_H
