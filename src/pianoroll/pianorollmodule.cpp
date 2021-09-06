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
#include "pianorollmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

#include "view/pianorollautomationeditor.h"
#include "view/pianorollview.h"
#include "view/pianorollruler.h"
#include "view/pianorollkeyboard.h"
#include "view/pianorollscrollbar.h"
#include "internal/pianorollcontroller.h"

using namespace mu::pianoroll;
using namespace mu::modularity;

static std::shared_ptr<PianorollController> s_pianorollController = std::make_shared<PianorollController>();

static void pianoroll_init_qrc()
{
    Q_INIT_RESOURCE(pianoroll);
}

std::string PianorollModule::moduleName() const
{
    return "pianoroll";
}

void PianorollModule::registerExports()
{
    ioc()->registerExport<IPianorollController>(moduleName(), s_pianorollController);
}

void PianorollModule::resolveImports()
{
}

void PianorollModule::registerResources()
{
    pianoroll_init_qrc();
}

void PianorollModule::registerUiTypes()
{
    qmlRegisterType<PianorollView>("MuseScore.Pianoroll", 1, 0, "PianorollView");
    qmlRegisterType<PianorollRuler>("MuseScore.Pianoroll", 1, 0, "PianorollRuler");
    qmlRegisterType<PianorollKeyboard>("MuseScore.Pianoroll", 1, 0, "PianorollKeyboard");
    qmlRegisterType<PianorollScrollbar>("MuseScore.Pianoroll", 1, 0, "PianorollScrollbar");
    qmlRegisterType<PianorollAutomationEditor>("MuseScore.Pianoroll", 1, 0, "PianorollAutomationEditor");

    auto ui = ioc()->resolve<ui::IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(pianoroll_QML_IMPORT);
    }
}

void PianorollModule::onInit(const framework::IApplication::RunMode& /*mode*/)
{
//    if (framework::IApplication::RunMode::Editor != mode) {
//        return;
//    }

    s_pianorollController->init();
}
