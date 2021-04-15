/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#include "pluginview.h"

#include "api/qmlplugin.h"
#include "log.h"

#include <QQmlComponent>

using namespace mu::plugins;
using namespace mu::framework;

PluginView::PluginView(const QUrl& url, QObject* parent)
    : QObject(parent)
{
    m_component = new QQmlComponent(engine(), url);
    m_qmlPlugin = qobject_cast<Ms::QmlPlugin*>(m_component->create());
}

PluginView::~PluginView()
{
    destroyView();
    delete m_component;
}

void PluginView::destroyView()
{
    if (m_view) {
        m_view->close();
        m_view->deleteLater();
    }
}

QQmlEngine* PluginView::engine() const
{
    return uiEngine()->qmlEngine();
}

QString PluginView::name() const
{
    IF_ASSERT_FAILED(m_qmlPlugin) {
        return QString();
    }

    QString menuPath = m_qmlPlugin->menuPath();
    return menuPath.mid(menuPath.lastIndexOf(".") + 1);
}

QString PluginView::description() const
{
    IF_ASSERT_FAILED(m_qmlPlugin) {
        return QString();
    }

    return m_qmlPlugin->description();
}

QVersionNumber PluginView::version() const
{
    IF_ASSERT_FAILED(m_qmlPlugin) {
        return QVersionNumber();
    }

    return QVersionNumber::fromString(m_qmlPlugin->version());
}

void PluginView::run()
{
    IF_ASSERT_FAILED(m_qmlPlugin && m_component) {
        return;
    }

    destroyView();
    m_view = new QQuickView(engine(), nullptr);
    m_view->setContent(QUrl(), m_component, m_qmlPlugin);
    m_view->setTitle(name());
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);

    connect(m_view, SIGNAL(closing(QQuickCloseEvent*)), this, SIGNAL(finished()));

    m_qmlPlugin->runPlugin();
    m_view->show();
}
