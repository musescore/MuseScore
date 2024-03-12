/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#include "extensionsuiengine.h"

#include <QQmlEngine>
#include <QQmlContext>

using namespace mu::extensions;

void ExtensionsUiEngine::setup(QQmlEngine* e)
{
    //! NOTE Needed for UI components, should not be used directly in extensions
    QObject* ui = dynamic_cast<QObject*>(uiEngine.get().get());
    e->rootContext()->setContextProperty("ui", ui);

    //! NOTE We prohibit importing default modules;
    //! only what is in the `qmlext` folder will be imported.
    e->addImportPath(":/qmlext");
}

QQmlEngine* ExtensionsUiEngine::engine()
{
    if (!m_engine) {
        m_engine = new QQmlEngine(this);
        setup(m_engine);
    }

    return m_engine;
}

QQmlEngine* ExtensionsUiEngine::qmlEngine() const
{
    return const_cast<ExtensionsUiEngine*>(this)->engine();
}
