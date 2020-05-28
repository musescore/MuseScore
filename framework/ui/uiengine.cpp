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

#include "uiengine.h"

#include <QApplication>
#include <QQmlEngine>
#include <QStringList>
#include <QDir>
#include <QQmlContext>

namespace Ms {

extern QString mscoreGlobalShare; //! FIXME Need to remove global variable

}

using namespace mu::framework;

const std::shared_ptr<UiEngine>& UiEngine::instance()
{
    struct make_shared_enabler : public UiEngine {};
    static std::shared_ptr<UiEngine> e = std::make_shared<make_shared_enabler>();
    return e;
}

UiEngine::UiEngine()
{

}

UiEngine::~UiEngine()
{
}

QQmlEngine* UiEngine::engine()
{
    if (_engine) {
        return _engine;
    }

    setup(new QQmlEngine(this));

    return _engine;
}

void UiEngine::moveQQmlEngine(QQmlEngine* e)
{
    setup(e);
}

void UiEngine::setup(QQmlEngine* e)
{
    delete _engine;
    delete _theme;

    _engine = e;

    _engine->rootContext()->setContextProperty("ui", this);
    _theme = new QmlTheme(QApplication::palette(), this);

#ifdef Q_OS_WIN
    QStringList importPaths;
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../qml"));
    importPaths.append(dir.absolutePath());
    _engine->setImportPathList(importPaths);
#endif
#ifdef Q_OS_MAC
    QStringList importPaths;
    QDir dir(mscoreGlobalShare + QString("/qml"));
    importPaths.append(dir.absolutePath());
    _engine->setImportPathList(importPaths);
#endif

    _engine->addImportPath(":/qml");
}

void UiEngine::updateTheme()
{
    if (!_engine) {
        return;
    }

    theme()->update(QApplication::palette());
}

QmlTheme* UiEngine::theme() const
{
    return _theme;
}

QQmlEngine* UiEngine::qmlEngine() const
{
    return const_cast<UiEngine*>(this)->engine();
}

void UiEngine::clearComponentCache()
{
    engine()->clearComponentCache();;
}
