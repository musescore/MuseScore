/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include <QObject>
#include <qqmlintegration.h>

#include "global/modularity/ioc.h"
#include "languages/ilanguagesservice.h"
#include "../iuiconfiguration.h"

#include "../api/themeapi.h"
#include "../view/qmltooltip.h"
#include "../view/qmltranslation.h"
#include "../view/qmlapi.h"
#include "../view/qmldataformatter.h"

#include "qmlnetworkaccessmanagerfactory.h"

#include "../iuiengine.h"

namespace muse::ui {
class UiEngine : public QObject, public IUiEngine, public Contextable
{
    Q_OBJECT

    QML_ELEMENT;
    QML_UNCREATABLE("Must be created in C++ only");

    Q_PROPERTY(api::ThemeApi * theme READ theme NOTIFY themeChanged)
    Q_PROPERTY(QmlToolTip * tooltip READ tooltip CONSTANT)
    Q_PROPERTY(QmlDataFormatter * df READ df CONSTANT)

    Q_PROPERTY(QQuickItem * rootItem READ rootItem WRITE setRootItem NOTIFY rootItemChanged)

    Q_PROPERTY(bool isEffectsAllowed READ isEffectsAllowed CONSTANT)
    Q_PROPERTY(bool isSystemDragSupported READ isSystemDragSupported CONSTANT)

    GlobalInject<languages::ILanguagesService> languagesService;
    GlobalInject<ui::IUiConfiguration> configuration;

public:
    UiEngine(const modularity::ContextPtr& iocCtx);
    ~UiEngine() override;

    void init();

    QmlApi* api() const;
    api::ThemeApi* theme() const;
    QmlToolTip* tooltip() const;
    QmlDataFormatter* df() const;

    Q_INVOKABLE Qt::KeyboardModifiers keyboardModifiers() const;
    Q_INVOKABLE Qt::LayoutDirection currentLanguageLayoutDirection() const;

    Q_INVOKABLE QColor colorWithAlphaF(const QColor& src, float alpha /* 0 - 1 */) const;
    Q_INVOKABLE QColor blendColors(const QColor& c1, const QColor& c2) const;
    Q_INVOKABLE QColor blendColors(const QColor& c1, const QColor& c2, float alpha) const;

    Q_INVOKABLE QStringList allTextFonts() const;

    // IUiEngine
    void updateTheme() override;
    QQmlApplicationEngine* qmlAppEngine() const override;
    QQmlEngine* qmlEngine() const override;
    void quit() override;
    void clearComponentCache() override;
    GraphicsApi graphicsApi() const override;
    QString graphicsApiName() const override;
    void addSourceImportPath(const QString& path) override;
    // ---

    QQuickItem* rootItem() const;

    bool isEffectsAllowed() const;
    bool isSystemDragSupported() const;

    // dev
    Q_INVOKABLE void sleep(int msec);

public slots:
    void setRootItem(QQuickItem* rootItem);

signals:
    void themeChanged(api::ThemeApi* theme);
    void rootItemChanged(QQuickItem* rootItem);

private:

    QQmlApplicationEngine* m_engine = nullptr;
    muse::api::JsApiEngine* m_apiEngine = nullptr;
    QStringList m_sourceImportPaths;
    api::ThemeApi* m_theme = nullptr;
    QmlTranslation* m_translation = nullptr;

    QmlApi* m_api = nullptr;
    QmlToolTip* m_tooltip = nullptr;
    QmlDataFormatter* m_dataFormatter = nullptr;
    QQuickItem* m_rootItem = nullptr;
    mutable int m_isEffectsAllowed = -1;

    QmlNetworkAccessManagerFactory* m_networkManagerFactory = nullptr;
};
}
