//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_PLUGINS_PLUGINVIEW_H
#define MU_PLUGINS_PLUGINVIEW_H

#include "plugins/pluginstypes.h"

#include "framework/ui/iuiengine.h"
#include "modularity/ioc.h"
#include "io/path.h"

namespace Ms {
class QmlPlugin;
}

class QQuickView;
class QQmlComponent;

namespace mu {
namespace plugins {
class PluginView : public QObject
{
    Q_OBJECT

    INJECT(plugins, framework::IUiEngine, uiEngine)

public:
    PluginView(const QUrl& url, QObject* parent = nullptr);
    ~PluginView();

    QString name() const;
    QString description() const;
    QVersionNumber version() const;

    void run();

signals:
    void finished();

private:
    QQmlEngine* engine() const;
    QString pluginName() const;

    void destroyView();

    Ms::QmlPlugin* m_qmlPlugin = nullptr;
    QQmlComponent* m_component = nullptr;
    QQuickView* m_view = nullptr;
};
}
}

#endif // MU_PLUGINS_PLUGINVIEW_H
