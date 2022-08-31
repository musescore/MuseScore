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

#ifndef MU_PLUGINS_PLUGINVIEW_H
#define MU_PLUGINS_PLUGINVIEW_H

#include <QQuickView>

#include "types/ret.h"

#include "modularity/ioc.h"
#include "framework/ui/iuiengine.h"
#include "ipluginsconfiguration.h"

class QQmlComponent;
class QQuickView;

namespace mu::engraving {
class QmlPlugin;
}

namespace mu::plugins {
class PluginView : public QObject
{
    Q_OBJECT

    INJECT(plugins, ui::IUiEngine, uiEngine)
    INJECT(plugins, IPluginsConfiguration, configuration)

public:
    PluginView(QObject* parent = nullptr);
    ~PluginView();

    Ret load(const QUrl& url);

    QString name() const;
    QString description() const;
    QVersionNumber version() const;
    QString thumbnailName() const;
    QString categoryCode() const;

    void run();

signals:
    void finished();

private:
    QQmlEngine* engine() const;
    QString pluginName() const;

    bool pluginHasUi() const;

    void destroyView();

    mu::engraving::QmlPlugin* m_qmlPlugin = nullptr;
    QQmlComponent* m_component = nullptr;
    QQuickView* m_view = nullptr;
};
}

#endif // MU_PLUGINS_PLUGINVIEW_H
