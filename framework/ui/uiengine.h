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

#include "interfaces/iuiengine.h"
#include "qmltheme.h"

class QQmlEngine;

namespace mu {
namespace framework {
class UiEngine : public QObject, public IUiEngine
{
    Q_OBJECT

    Q_PROPERTY(QmlTheme * theme READ theme NOTIFY themeChanged)

public:
    ~UiEngine();

    static const std::shared_ptr<UiEngine>& instance();

    QmlTheme* theme() const;

    // IUiEngine
    void updateTheme() override;
    QQmlEngine* qmlEngine() const override;
    void clearComponentCache() override;
    // ---

    void moveQQmlEngine(QQmlEngine* e);

signals:
    void themeChanged(QmlTheme* theme);

private:

    UiEngine();

    QQmlEngine* engine();
    void setup(QQmlEngine* e);

    QQmlEngine* _engine = nullptr;
    QmlTheme* _theme = nullptr;
};
}
}

#endif // MU_FRAMEWORK_UIENGINE_H
