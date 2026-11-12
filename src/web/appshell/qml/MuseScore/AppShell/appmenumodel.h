/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
#pragma once

#include <qqmlintegration.h>

#include "uicomponents/qml/Muse/UiComponents/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "global/iglobalconfiguration.h"
#include "project/iprojectsession.h"
#include "ui/imainwindow.h"
#include "ui/inavigationcontroller.h"
#include "ui/iuiconfiguration.h"

#include "notation/inotationundostack.h"

namespace mu::appshell {
class AppMenuModel : public muse::uicomponents::AbstractMenuModel
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    muse::GlobalInject<muse::ui::IUiConfiguration> uiConfiguration;
    muse::GlobalInject<muse::IGlobalConfiguration> globalConfiguration;
    muse::ContextInject<mu::project::IProjectSession> projectSession = { this };
    muse::ContextInject<muse::ui::IMainWindow> mainWindow = { this };
    muse::ContextInject<muse::ui::INavigationController> navigationController = { this };

public:
    explicit AppMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;
    Q_INVOKABLE bool isGlobalMenuAvailable();

private:
    void setupConnections();

    muse::uicomponents::MenuItem* makeFileMenu();
    muse::uicomponents::MenuItem* makeEditMenu();
    muse::uicomponents::MenuItem* makeViewMenu();
    muse::uicomponents::MenuItem* makeAddMenu();
    muse::uicomponents::MenuItem* makeFormatMenu();
    muse::uicomponents::MenuItem* makeToolsMenu();
    muse::uicomponents::MenuItem* makeDiagnosticsMenu();

    muse::uicomponents::MenuItemList makeNotesItems();
    muse::uicomponents::MenuItemList makeIntervalsItems();
    muse::uicomponents::MenuItemList makeTupletsItems();
    muse::uicomponents::MenuItemList makeMeasuresItems();
    muse::uicomponents::MenuItemList makeFramesItems();
    muse::uicomponents::MenuItemList makeTextItems();
    muse::uicomponents::MenuItemList makeLinesItems();
    muse::uicomponents::MenuItemList makeChordAndFretboardDiagramsItems();

    mu::notation::INotationUndoStackPtr undoStack() const;
    void updateUndoRedoItems();
};
}
