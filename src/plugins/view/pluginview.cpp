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

#include "pluginview.h"

#include <QQmlComponent>

#include "pluginserrors.h"

#include "api/qmlplugin.h"

#include "log.h"

using namespace mu::plugins;

PluginView::PluginView(QObject* parent)
    : QObject(parent)
{
}

PluginView::~PluginView()
{
    destroyView();

    if (m_component) {
        delete m_component;
    }

    if (m_qmlPlugin) {
        delete m_qmlPlugin;
    }
}

mu::Ret PluginView::load(const QUrl& url)
{
    m_component = new QQmlComponent(engine(), url);

    if (!m_component->isReady()) {
        LOGE() << "Failed to load QML plugin from " << url;
        LOGE() << m_component->errors().at(0).toString();
        return make_ret(Err::PluginLoadError);
    }

    m_qmlPlugin = qobject_cast<mu::engraving::QmlPlugin*>(m_component->create());

    if (!m_qmlPlugin) {
        LOGE() << "Failed to create instance of QML plugin from " << url;
        return make_ret(Err::PluginLoadError);
    }

    connect(m_qmlPlugin, &mu::engraving::QmlPlugin::closeRequested, [this]() {
        if (m_view && m_view->isVisible()) {
            m_view->close();
        }
    });

    return make_ok();
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

bool PluginView::pluginHasUi() const
{
    IF_ASSERT_FAILED(m_qmlPlugin) {
        return false;
    }

    return m_qmlPlugin->pluginType() == "dialog";
}

QString PluginView::name() const
{
    IF_ASSERT_FAILED(m_qmlPlugin) {
        return QString();
    }

    return m_qmlPlugin->title();
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

QString PluginView::thumbnailName() const
{
    IF_ASSERT_FAILED(m_qmlPlugin) {
        return QString();
    }

    return m_qmlPlugin->thumbnailName();
}

QString PluginView::categoryCode() const
{
    IF_ASSERT_FAILED(m_qmlPlugin) {
        return QString();
    }

    return m_qmlPlugin->categoryCode();
}

mu::engraving::QmlPlugin* PluginView::qmlPlugin() const
{
    return m_qmlPlugin;
}

void PluginView::run()
{
    IF_ASSERT_FAILED(m_qmlPlugin && m_component) {
        return;
    }

    if (!pluginHasUi()) {
        m_qmlPlugin->runPlugin();
        return;
    }

    destroyView();
    m_view = new QQuickView(engine(), nullptr);
    m_view->setContent(QUrl(), m_component, m_qmlPlugin);
    m_view->setTitle(name());
    m_view->setColor(configuration()->viewBackgroundColor());
    m_view->setResizeMode(QQuickView::SizeRootObjectToView);

    // TODO: Can't use new `connect` syntax because the QQuickView::closing
    // has a parameter of type QQuickCloseEvent, which is not public, so we
    // can't include any header for it and it will always be an incomplete
    // type, which is not allowed for the new `connect` syntax.
    //connect(m_view, &QQuickView::closing, this, &PluginView::finished);
    connect(m_view, SIGNAL(closing(QQuickCloseEvent*)), this, SIGNAL(finished()));

    m_view->show();

    m_qmlPlugin->runPlugin();
}
