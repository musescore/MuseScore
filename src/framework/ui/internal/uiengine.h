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

#ifndef MUSE_UI_UIENGINE_H
#define MUSE_UI_UIENGINE_H

#include <QObject>
#include <memory>

#include "../iuiengine.h"
#include "../api/themeapi.h"
#include "../view/qmltooltip.h"
#include "../view/qmltranslation.h"
#include "../view/interactiveprovider.h"
#include "../view/qmlapi.h"
#include "../view/qmldataformatter.h"

#include "languages/ilanguagesservice.h"

namespace muse::ui {
class QmlApiEngine;
class UiEngine : public QObject, public IUiEngine, public Injectable
{
    Q_OBJECT

    Q_PROPERTY(api::ThemeApi * theme READ theme NOTIFY themeChanged)
    Q_PROPERTY(QmlToolTip * tooltip READ tooltip CONSTANT)
    Q_PROPERTY(QmlDataFormatter * df READ df CONSTANT)

    Q_PROPERTY(QQuickItem * rootItem READ rootItem WRITE setRootItem NOTIFY rootItemChanged)

    Q_PROPERTY(bool isEffectsAllowed READ isEffectsAllowed CONSTANT)

    // for internal use
    Q_PROPERTY(InteractiveProvider * _interactiveProvider READ interactiveProvider_property CONSTANT)

    GlobalInject<languages::ILanguagesService> languagesService;

public:
    UiEngine(const modularity::ContextPtr& iocCtx);
    ~UiEngine() override;

    void init();

    QmlApi* api() const;
    api::ThemeApi* theme() const;
    QmlToolTip* tooltip() const;
    QmlDataFormatter* df() const;

    InteractiveProvider* interactiveProvider_property() const;
    std::shared_ptr<InteractiveProvider> interactiveProvider() const;

    Q_INVOKABLE Qt::KeyboardModifiers keyboardModifiers() const;
    Q_INVOKABLE Qt::LayoutDirection currentLanguageLayoutDirection() const;

    Q_INVOKABLE QColor colorWithAlphaF(const QColor& src, float alpha /* 0 - 1 */) const;
    Q_INVOKABLE QColor blendColors(const QColor& c1, const QColor& c2) const;
    Q_INVOKABLE QColor blendColors(const QColor& c1, const QColor& c2, float alpha) const;

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

public slots:
    void setRootItem(QQuickItem* rootItem);

signals:
    void themeChanged(api::ThemeApi* theme);
    void rootItemChanged(QQuickItem* rootItem);

private:

    QQmlApplicationEngine* m_engine = nullptr;
    QmlApiEngine* m_apiEngine = nullptr;
    QStringList m_sourceImportPaths;
    api::ThemeApi* m_theme = nullptr;
    QmlTranslation* m_translation = nullptr;

    std::shared_ptr<InteractiveProvider> m_interactiveProvider = nullptr;

    QmlApi* m_api = nullptr;
    QmlToolTip* m_tooltip = nullptr;
    QmlDataFormatter* m_dataFormatter = nullptr;
    QQuickItem* m_rootItem = nullptr;
    mutable int m_isEffectsAllowed = -1;
};
}

#endif // MUSE_UI_UIENGINE_H
