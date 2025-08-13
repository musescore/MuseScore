/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "modularity/ioc.h"
#include "iappshellconfiguration.h"
#include "appshell/internal/istartupscenario.h"

namespace mu::appshell {
class WelcomeDialogModel : public QObject, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(size_t count READ count NOTIFY itemsChanged)

    Q_PROPERTY(QVariant currentItem READ currentItem NOTIFY currentItemChanged)
    Q_PROPERTY(size_t currentIndex READ currentIndex NOTIFY currentItemChanged)

    Q_PROPERTY(bool showOnStartup READ showOnStartup WRITE setShowOnStartup NOTIFY showOnStartupChanged)

    Inject<IAppShellConfiguration> configuration = { this };
    Inject<IStartupScenario> startupScenario = { this };

public:
    WelcomeDialogModel();

    Q_INVOKABLE void init();

    size_t count() const { return m_items.size(); }

    QVariantMap currentItem() const;
    size_t currentIndex() const { return m_currentIndex; }

    Q_INVOKABLE void nextItem();
    Q_INVOKABLE void prevItem();

    bool showOnStartup() const;
    void setShowOnStartup(bool show);

signals:
    void itemsChanged();
    void currentItemChanged();
    void showOnStartupChanged();

private:
    bool hasPrev() const { return m_currentIndex > 0; }
    bool hasNext() const { return m_currentIndex < count() - 1; }

    QList<QVariantMap> m_items;
    size_t m_currentIndex = 0;
};
}
