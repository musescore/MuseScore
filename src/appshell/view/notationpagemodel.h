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
#pragma once

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
class NotationPageModel : public QObject, public muse::LazyInjectable, public muse::async::Asyncable, public muse::actions::Actionable
{
    Q_OBJECT

    Q_PROPERTY(bool isNavigatorVisible READ isNavigatorVisible NOTIFY isNavigatorVisibleChanged)
    Q_PROPERTY(bool isBraillePanelVisible READ isBraillePanelVisible NOTIFY isBraillePanelVisibleChanged)

    muse::LazyInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::LazyInject<muse::dock::IDockWindowProvider> dockWindowProvider = { this };
    muse::LazyInject<muse::extensions::IExtensionsProvider> extensionsProvider = { this };
    muse::LazyInject<context::IGlobalContext> globalContext = { this };
    muse::LazyInject<notation::INotationConfiguration> notationConfiguration = { this };
    muse::LazyInject<braille::IBrailleConfiguration> brailleConfiguration = { this };
    muse::LazyInject<IAppShellConfiguration> configuration = { this };

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

    void scheduleUpdateDrumsetPanelVisibility();
    void doUpdateDrumsetPanelVisibility();

    void scheduleUpdatePercussionPanelVisibility();
    void doUpdatePercussionPanelVisibility();

    void scheduleUpdateExtensionsToolBarVisibility();
    void doUpdateExtensionsToolBarVisibility();

    bool m_inited = false;
    bool m_updateDrumsetPanelVisibilityScheduled = false;
    bool m_updatePercussionPanelVisibilityScheduled = false;
    bool m_updateExtensionsToolBarVisibilityScheduled = false;
};
}
