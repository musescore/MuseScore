/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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
#include "notationscenemodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iinteractiveuriregister.h"
#include "ui/iuiactionsregister.h"

#include "internal/notationactioncontroller.h"
#include "internal/midiinputoutputcontroller.h"
#include "internal/notationuiactions.h"

#include "widgets/breaksdialog.h"
#include "widgets/editstaff.h"
#include "widgets/editstringdata.h"
#include "widgets/editstyle.h"
#include "widgets/measureproperties.h"
#include "widgets/pagesettings.h"
#include "widgets/realizeharmonydialog.h"
#include "widgets/selectdialog.h"
#include "widgets/selectnotedialog.h"
#include "widgets/stafftextpropertiesdialog.h"
#include "widgets/transposedialog.h"
#include "widgets/tupletdialog.h"

using namespace mu::notation;
using namespace muse;
using namespace muse::modularity;
using namespace muse::ui;
using namespace muse::actions;
using namespace muse::uicomponents;

std::string NotationSceneModule::moduleName() const
{
    return "notationscene";
}

void NotationSceneModule::registerExports()
{
    m_actionController = std::make_shared<NotationActionController>();
    m_notationUiActions = std::make_shared<NotationUiActions>(m_actionController);
    m_midiInputOutputController = std::make_shared<MidiInputOutputController>();
}

void NotationSceneModule::resolveImports()
{
    auto ar = ioc()->resolve<IUiActionsRegister>(moduleName());
    if (ar) {
        ar->reg(m_notationUiActions);
    }

    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerWidgetUri<EditStyle>(Uri("musescore://notation/style"));
        ir->registerWidgetUri<PageSettings>(Uri("musescore://notation/pagesettings"));
        ir->registerWidgetUri<MeasurePropertiesDialog>(Uri("musescore://notation/measureproperties"));
        ir->registerWidgetUri<BreaksDialog>(Uri("musescore://notation/breaks"));
        ir->registerWidgetUri<EditStaff>(Uri("musescore://notation/staffproperties"));
        ir->registerWidgetUri<EditStringData>(Uri("musescore://notation/editstrings"));
        ir->registerWidgetUri<TransposeDialog>(Uri("musescore://notation/transpose"));
        ir->registerWidgetUri<SelectNoteDialog>(Uri("musescore://notation/selectnote"));
        ir->registerWidgetUri<SelectDialog>(Uri("musescore://notation/selectelement"));
        ir->registerWidgetUri<TupletDialog>(Uri("musescore://notation/othertupletdialog"));
        ir->registerWidgetUri<StaffTextPropertiesDialog>(Uri("musescore://notation/stafftextproperties"));
        ir->registerWidgetUri<RealizeHarmonyDialog>(Uri("musescore://notation/realizechordsymbols"));

        ir->registerQmlUri(Uri("musescore://notation/parts"), "MuseScore.NotationScene", "PartsDialog");
        ir->registerQmlUri(Uri("musescore://notation/selectmeasurescount"), "MuseScore.NotationScene", "SelectMeasuresCountDialog");
        ir->registerQmlUri(Uri("musescore://notation/editgridsize"), "MuseScore.NotationScene", "EditGridSizeDialog");
        ir->registerQmlUri(Uri("musescore://notation/percussionpanelpadswap"), "MuseScore.NotationScene", "PercussionPanelPadSwapDialog");
        ir->registerQmlUri(Uri("musescore://notation/editpercussionshortcut"), "MuseScore.NotationScene", "EditPercussionShortcutDialog");
    }
}

void NotationSceneModule::onInit(const IApplication::RunMode& mode)
{
    m_actionController->init();
    m_notationUiActions->init();

    if (mode == IApplication::RunMode::GuiApp) {
        m_midiInputOutputController->init();
    }
}
