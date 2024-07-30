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

using namespace muse::learn;
using namespace muse::modularity;

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
    m_learnConfiguration = std::make_shared<LearnConfiguration>(iocContext());
    m_learnService = std::make_shared<LearnService>(iocContext());

    ioc()->registerExport<ILearnConfiguration>(moduleName(), m_learnConfiguration);
    ioc()->registerExport<ILearnService>(moduleName(), m_learnService);
}

void LearnModule::registerResources()
{
    learn_init_qrc();
}

void LearnModule::registerUiTypes()
{
    qmlRegisterType<LearnPageModel>("Muse.Learn", 1, 0, "LearnPageModel");
}

void LearnModule::onInit(const IApplication::RunMode&)
{
    m_learnConfiguration->init();
}

void LearnModule::onDelayedInit()
{
    m_learnService->refreshPlaylists();
}
