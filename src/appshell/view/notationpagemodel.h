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
#ifndef MU_APPSHELL_NOTATIONPAGEMODEL_H
#define MU_APPSHELL_NOTATIONPAGEMODEL_H

#include <QQuickItem>

#include "async/asyncable.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "dockwindow/idockwindowprovider.h"
#include "extensions/iextensionsprovider.h"
#include "context/iglobalcontext.h"
#include "notation/inotationconfiguration.h"
#include "braille/ibrailleconfiguration.h"
#include "iappshellconfiguration.h"

namespace mu::appshell {
class NotationPageModel : public QObject, public muse::Injectable, public muse::async::Asyncable, public muse::actions::Actionable
{
    Q_OBJECT

    Q_PROPERTY(bool isNavigatorVisible READ isNavigatorVisible NOTIFY isNavigatorVisibleChanged)
    Q_PROPERTY(bool isBraillePanelVisible READ isBraillePanelVisible NOTIFY isBraillePanelVisibleChanged)

    Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    Inject<muse::dock::IDockWindowProvider> dockWindowProvider = { this };
    Inject<muse::extensions::IExtensionsProvider> extensionsProvider = { this };
    Inject<context::IGlobalContext> globalContext = { this };
    Inject<notation::INotationConfiguration> notationConfiguration = { this };
    Inject<braille::IBrailleConfiguration> brailleConfiguration = { this };
    Inject<IAppShellConfiguration> configuration = { this };

public:
    explicit NotationPageModel(QObject* parent = nullptr);

    bool isNavigatorVisible() const;
    bool isBraillePanelVisible() const;

    Q_INVOKABLE void init();

    Q_INVOKABLE QString notationToolBarName() const;
    Q_INVOKABLE QString playbackToolBarName() const;
    Q_INVOKABLE QString undoRedoToolBarName() const;
    Q_INVOKABLE QString noteInputBarName() const;
    Q_INVOKABLE QString extensionsToolBarName() const;

    Q_INVOKABLE QString palettesPanelName() const;
    Q_INVOKABLE QString layoutPanelName() const;
    Q_INVOKABLE QString inspectorPanelName() const;
    Q_INVOKABLE QString selectionFiltersPanelName() const;
    Q_INVOKABLE QString undoHistoryPanelName() const;

    Q_INVOKABLE QString mixerPanelName() const;
    Q_INVOKABLE QString pianoKeyboardPanelName() const;
    Q_INVOKABLE QString timelinePanelName() const;
    Q_INVOKABLE QString drumsetPanelName() const;
    Q_INVOKABLE QString percussionPanelName() const;

    Q_INVOKABLE QString statusBarName() const;

signals:
    void isNavigatorVisibleChanged();
    void isBraillePanelVisibleChanged();

private:
    void onNotationChanged();

    void toggleDock(const QString& name);

    void updateDrumsetPanelVisibility();
    void updatePercussionPanelVisibility();
    void updateExtensionsToolBarVisibility();
};
}

#endif // MU_APPSHELL_NOTATIONPAGEMODEL_H
