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
#ifndef MUSE_EXTENSIONS_EXTENSIONSUIENGINE_H
#define MUSE_EXTENSIONS_EXTENSIONSUIENGINE_H

#include <QObject>

#include "../iextensionsuiengine.h"

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "../api/extapi.h"
#include "../api/v1/extapiv1.h"

namespace muse::extensions {
class QmlApiEngine;
class ExtensionsUiEngine : public QObject, public IExtensionsUiEngine, public Injectable
{
    Q_OBJECT

    Inject<ui::IUiEngine> uiEngine = { this };

public:
    ExtensionsUiEngine(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    ~ExtensionsUiEngine();

    QQmlEngine* qmlEngine() const;
    QQmlEngine* qmlEngineApiV1() const;

private:

    QQmlEngine* engine();
    void setup();

    QQmlEngine* m_engine = nullptr;
    QmlApiEngine* m_apiEngine = nullptr;
    api::ExtApi* m_api = nullptr;

    // api v1

    QQmlEngine* engineV1();
    void setupV1();

    QQmlEngine* m_engineV1 = nullptr;
    QmlApiEngine* m_apiEngineV1 = nullptr;
    apiv1::ExtApiV1* m_apiV1 = nullptr;
};
}

#endif // MUSE_EXTENSIONS_EXTENSIONSUIENGINE_H
