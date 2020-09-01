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

#ifndef MU_FRAMEWORK_UIENGINE_H
#define MU_FRAMEWORK_UIENGINE_H

#include <QObject>
#include <memory>

#include "../iuiengine.h"
#include "../view/qmltheme.h"
#include "../view/qmltooltip.h"
#include "../view/qmltranslation.h"
#include "../view/interactiveprovider.h"
#include "../view/qmlapi.h"

class QQmlEngine;

namespace mu {
namespace framework {
class UiEngine : public QObject, public IUiEngine
{
    Q_OBJECT

    Q_PROPERTY(QmlTheme * theme READ theme NOTIFY themeChanged)
    Q_PROPERTY(QmlToolTip * tooltip READ tooltip CONSTANT)

    // for internal use
    Q_PROPERTY(InteractiveProvider * _interactiveProvider READ interactiveProvider_property CONSTANT)

public:
    ~UiEngine();

    static UiEngine* instance();

    QmlApi* api() const;
    QmlTheme* theme() const;
    QmlToolTip* tooltip() const;
    InteractiveProvider* interactiveProvider_property() const;
    std::shared_ptr<InteractiveProvider> interactiveProvider() const;

    Q_INVOKABLE Qt::KeyboardModifiers keyboardModifiers() const;

    // IUiEngine
    void updateTheme() override;
    QQmlEngine* qmlEngine() const override;
    void clearComponentCache() override;
    void addSourceImportPath(const QString& path) override;
    // ---

    void moveQQmlEngine(QQmlEngine* e);

signals:
    void themeChanged(QmlTheme* theme);

private:
    UiEngine();

    QQmlEngine* engine();
    void setup(QQmlEngine* e);

    QQmlEngine* m_engine = nullptr;
    QStringList m_sourceImportPaths;
    QmlTheme* m_theme = nullptr;
    QmlTranslation* m_translation = nullptr;
    std::shared_ptr<InteractiveProvider> m_interactiveProvider = nullptr;
    QmlApi* m_api = nullptr;
    QmlToolTip* m_tooltip = nullptr;
};
}
}

#endif // MU_FRAMEWORK_UIENGINE_H
