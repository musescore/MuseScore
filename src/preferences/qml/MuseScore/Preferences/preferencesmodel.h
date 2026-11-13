/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include <qqmlintegration.h>

#include <QAbstractListModel>

#include "async/asyncable.h"
#include "ui/view/iconcodes.h"

#include "modularity/ioc.h"
#include "appshell/iappshellconfiguration.h"
#include "interactive/iinteractive.h"
#include "shortcuts_v2/icommandshortcutsregister.h"
#include "midiremote/imidiremote.h"

#include "preferencepageitem.h"

namespace mu::preferences {
class PreferencesModel : public QAbstractListModel, public muse::Contextable, public muse::async::Asyncable
{
    Q_OBJECT
    QML_ELEMENT;

    Q_PROPERTY(QString currentPageId READ currentPageId WRITE setCurrentPageId NOTIFY currentPageIdChanged)

    muse::GlobalInject<appshell::IAppShellConfiguration> configuration;
    muse::GlobalInject<muse::shortcuts::ICommandShortcutsRegister> commandShortcutsRegister;
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::midiremote::IMidiRemote> midiRemote = { this };

public:
    explicit PreferencesModel(QObject* parent = nullptr);
    ~PreferencesModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString currentPageId() const;

    Q_INVOKABLE void load(const QString& currentPageId);
    Q_INVOKABLE void askForConfirmationOfPreferencesReset();
    Q_INVOKABLE void resetFactorySettings();
    Q_INVOKABLE void apply();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void selectRow(int row);

    Q_INVOKABLE QVariantList availablePages() const;

public slots:
    void setCurrentPageId(QString currentPageId);

signals:
    void userAgreesToResetPreferences();
    void currentPageIdChanged(QString currentPageId);

private:

    enum RoleNames {
        ItemRole = Qt::UserRole + 1
    };

    PreferencePageItem* makeItem(const QString& id, const QString& title, muse::ui::IconCode::Code icon, const QString& path);

    QList<PreferencePageItem*> m_items;
    QString m_currentPageId;
};
}
