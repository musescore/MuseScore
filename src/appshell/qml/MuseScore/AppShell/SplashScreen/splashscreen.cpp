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

#include "splashscreen.h"

#include <QQmlContext>
#include <QQuickItem>
#include <QScreen>
#include <QShowEvent>
#include <QSurfaceFormat>

#include "modularity/ioc.h"

#include "loadingscreenmodel.h"
#include "newinstanceloadingscreenmodel.h"

using namespace mu::appshell;

SplashScreen::SplashScreen(SplashScreen::SplashScreenType type, bool forNewScore, const QString& openingFileName,
                           const muse::modularity::ContextPtr& iocContext)
    : QQuickView()
{
    setFlags(Qt::SplashScreen);
    setModality(Qt::ApplicationModal);

    if (iocContext) {
        muse::QmlIoCContext* iocCtx = new muse::QmlIoCContext(engine()->rootContext());
        iocCtx->ctx = iocContext;
        engine()->rootContext()->setContextProperty("ioc_context", QVariant::fromValue(iocCtx));
    }

    switch (type) {
    case SplashScreen::Default: {
        LoadingScreenModel* model = new LoadingScreenModel(iocContext, this);
        engine()->rootContext()->setContextProperty("loadingScreenModel", model);
        setSource(QUrl("qrc:/qt/qml/MuseScore/AppShell/SplashScreen/LoadingScreenView.qml"));
        m_contentItem = qobject_cast<QQuickItem*>(rootObject());
        break;
    }
    case SplashScreen::ForNewInstance: {
        NewInstanceLoadingScreenModel* model = new NewInstanceLoadingScreenModel(forNewScore, openingFileName,
                                                                                 iocContext, this);
        engine()->rootContext()->setContextProperty("newInstanceLoadingScreenModel", model);
        setSource(QUrl("qrc:/qt/qml/MuseScore/AppShell/SplashScreen/NewInstanceLoadingScreenView.qml"));
        m_contentItem = qobject_cast<QQuickItem*>(rootObject());
        break;
    }
    }

    if (m_contentItem) {
        setSize(QSize(m_contentItem->width(), m_contentItem->height()));
    }
}

void SplashScreen::setSize(const QSize& size)
{
    resize(size);

    if (screen()) {
        setGeometry(screen()->geometry().center().x() - size.width() / 2,
                    screen()->geometry().center().y() - size.height() / 2,
                    size.width(),
                    size.height());
    }
}
