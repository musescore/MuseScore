/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "testing/environment.h"

#include <QQmlEngine>

#include "plugins/pluginsmodule.h"

#include "modularity/ioc.h"
#include "mocks/uienginemock.h"

#include "log.h"

static mu::testing::SuiteEnvironment plugins_env(
{
    new mu::plugins::PluginsModule()
},
    []() {
    auto uiEngine = std::make_shared<testing::NiceMock<mu::plugins::UiEngineMock> >();

    auto qmlEngine = new QQmlEngine(uiEngine.get());

    mu::modularity::ioc()->registerExport<mu::ui::IUiEngine>("ui", uiEngine);

    ON_CALL(*uiEngine, qmlEngine()).WillByDefault(testing::Return(qmlEngine));

    QObject::connect(qmlEngine, &QQmlEngine::warnings, [](const QList<QQmlError>& warnings) {
        for (const QQmlError& e : warnings) {
            LOGE() << "error: " << e.toString() << "\n";
        }
    });
});
