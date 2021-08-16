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

using namespace mu::pianoroll;
using namespace mu::modularity;

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
    //qmlRegisterType<PianoRollView>("MuseScore.Pianoroll", 1, 0, "PianoRollView");

    auto ui = ioc()->resolve<ui::IUiEngine>(moduleName());
    if (ui) {
        ui->addSourceImportPath(pianoroll_QML_IMPORT);
    }
}

void PianorollModule::onInit(const framework::IApplication::RunMode& /*mode*/)
{
    //if (framework::IApplication::RunMode::Editor != mode) {
    //    return;
    //}
}
