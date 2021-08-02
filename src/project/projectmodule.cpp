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
#include "projectmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "internal/projectcreator.h"
#include "internal/notationreadersregister.h"
#include "internal/notationwritersregister.h"
#include "internal/projectautosaver.h"
#include "internal/filescorecontroller.h"
#include "internal/projectuiactions.h"
#include "internal/projectconfiguration.h"

#ifdef Q_OS_MAC
#include "internal/platform/macos/macosrecentfilescontroller.h"
#elif defined (Q_OS_WIN)
#include "internal/platform/windows/windowsrecentfilescontroller.h"
#else
#include "internal/platform/stub/stubrecentfilescontroller.h"
#endif

#include "ui/iuiactionsregister.h"

using namespace mu::project;
using namespace mu::modularity;

static std::shared_ptr<ProjectConfiguration> s_configuration = std::make_shared<ProjectConfiguration>();
static std::shared_ptr<FileScoreController> s_fileController = std::make_shared<FileScoreController>();

std::string ProjectModule::moduleName() const
{
    return "project";
}

void ProjectModule::registerExports()
{
    ioc()->registerExport<IProjectConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IProjectCreator>(moduleName(), new ProjectCreator());
    ioc()->registerExport<INotationReadersRegister>(moduleName(), new NotationReadersRegister());
    ioc()->registerExport<INotationWritersRegister>(moduleName(), new NotationWritersRegister());
    ioc()->registerExport<IFileScoreController>(moduleName(), s_fileController);

#ifdef Q_OS_MAC
    ioc()->registerExport<IPlatformRecentFilesController>(moduleName(), new MacOSRecentFilesController());
#elif defined (Q_OS_WIN)
    ioc()->registerExport<IPlatformRecentFilesController>(moduleName(), new WindowsRecentFilesController());
#else
    ioc()->registerExport<IPlatformRecentFilesController>(moduleName(), new StubRecentFilesController());
#endif
}

void ProjectModule::resolveImports()
{
    auto ar = ioc()->resolve<ui::IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(std::make_shared<UserScoresUiActions>(s_fileController));
    }
}

void ProjectModule::registerUiTypes()
{
}

void ProjectModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Editor != mode) {
        return;
    }

    static ProjectAutoSaver autoSaver;
    autoSaver.init();

    s_fileController->init();
}
