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
#include "learnmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"

#include "internal/learnconfiguration.h"
#include "internal/learnservice.h"

#include "view/learnpagemodel.h"

using namespace mu::learn;
using namespace mu::framework;
using namespace mu::modularity;

static std::shared_ptr<LearnConfiguration> s_learnConfiguration = std::make_shared<LearnConfiguration>();
static std::shared_ptr<LearnService> s_learnService = std::make_shared<LearnService>();

static void learn_init_qrc()
{
    Q_INIT_RESOURCE(learn);
}

std::string LearnModule::moduleName() const
{
    return "learn";
}

void LearnModule::registerExports()
{
    ioc()->registerExport<ILearnConfiguration>(moduleName(), s_learnConfiguration);
    ioc()->registerExport<ILearnService>(moduleName(), s_learnService);
}

void LearnModule::registerResources()
{
    learn_init_qrc();
}

void LearnModule::registerUiTypes()
{
    qmlRegisterType<LearnPageModel>("MuseScore.Learn", 1, 0, "LearnPageModel");
}

void LearnModule::onDelayedInit()
{
    s_learnService->refreshPlaylists();
}
