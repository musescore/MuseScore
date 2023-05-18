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
#include "plugins/ipluginsconfiguration.h"

#include "uicomponents/view/dialogview.h"

class QQmlComponent;
class QQuickView;

namespace mu::plugins {
class QmlPlugin;

class PluginView : public QObject
{
    Q_OBJECT

    INJECT(ui::IUiEngine, uiEngine)
    INJECT(IPluginsConfiguration, configuration)

public:
    PluginView(QObject* parent = nullptr);
    ~PluginView();

    Ret load(const QUrl& url);

    QString name() const;
    QString description() const;
    QVersionNumber version() const;
    QString thumbnailName() const;
    QString categoryCode() const;

    QmlPlugin* qmlPlugin() const;

    void run();

signals:
    void finished();

private:
    QQmlEngine* engine() const;
    QString pluginName() const;

    bool pluginHasUi() const;

    void destroyView();

    QmlPlugin* m_qmlPlugin = nullptr;
    QQmlComponent* m_component = nullptr;
    uicomponents::DialogView* m_dialogView = nullptr;
};
}

#endif // MU_PLUGINS_PLUGINVIEW_H
