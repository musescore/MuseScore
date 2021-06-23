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
#ifndef MU_APPSHELL_NOTATIONPAGEMODEL_H
#define MU_APPSHELL_NOTATIONPAGEMODEL_H

#include <QQuickItem>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "actions/actionable.h"
#include "actions/iactionsdispatcher.h"
#include "inotationpagestate.h"

namespace mu::dock {
class DockWindow;
}

namespace mu::appshell {
class NotationPageModel : public QObject, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(appshell, INotationPageState, pageState)
    INJECT(appshell, actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(bool isNavigatorVisible READ isNavigatorVisible NOTIFY isNavigatorVisibleChanged)

public:
    explicit NotationPageModel(QObject* parent = nullptr);

    bool isNavigatorVisible() const;

    Q_INVOKABLE void setNotationToolBarDockName(const QString& dockName);
    Q_INVOKABLE void setPlaybackToolBarDockName(const QString& dockName);
    Q_INVOKABLE void setUndoRedoToolBarDockName(const QString& dockName);
    Q_INVOKABLE void setNoteInputBarDockName(const QString& dockName);

    Q_INVOKABLE void setPalettePanelDockName(const QString& dockName);
    Q_INVOKABLE void setInstrumentsPanelDockName(const QString& dockName);
    Q_INVOKABLE void setInspectorPanelDockName(const QString& dockName);

    Q_INVOKABLE void setPianoRollDockName(const QString& dockName);
    Q_INVOKABLE void setMixerDockName(const QString& dockName);

    Q_INVOKABLE void setStatusBarDockName(const QString& dockName);

    Q_INVOKABLE void init(QQuickItem* dockWindow);

signals:
    void isNavigatorVisibleChanged();

private:
    void setPanelDockName(PanelType type, const QString& dockName);
    void togglePanel(PanelType type);

    QMap<PanelType, QString /* dockName */> m_panelTypeToDockName;
    dock::DockWindow* m_window = nullptr;
};
}

#endif // MU_APPSHELL_NOTATIONPAGEMODEL_H
