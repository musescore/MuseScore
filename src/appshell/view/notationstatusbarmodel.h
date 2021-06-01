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

#ifndef MU_NOTATION_NOTATIONSTATUSBARMODEL_H
#define MU_NOTATION_NOTATIONSTATUSBARMODEL_H

#include <QObject>

#include "async/asyncable.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"
#include "workspace/iworkspaceconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "context/iglobalcontext.h"

namespace mu::appshell {
class NotationStatusBarModel : public QObject, public async::Asyncable, public actions::Actionable
{
    Q_OBJECT

    INJECT(notation, context::IGlobalContext, context)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)
    INJECT(notation, ui::IUiActionsRegister, actionsRegister)
    INJECT(notation, workspace::IWorkspaceConfiguration, workspaceConfiguration)
    INJECT(notation, notation::INotationConfiguration, notationConfiguration)

    Q_PROPERTY(QString accessibilityInfo READ accessibilityInfo NOTIFY accessibilityInfoChanged)
    Q_PROPERTY(QString currentWorkspaceName READ currentWorkspaceName NOTIFY currentWorkspaceNameChanged)
    Q_PROPERTY(bool concertPitchEnabled READ concertPitchEnabled NOTIFY concertPitchEnabledChanged)
    Q_PROPERTY(QVariant currentViewMode READ currentViewMode NOTIFY currentViewModeChanged)
    Q_PROPERTY(QVariantList availableViewModeList READ availableViewModeList NOTIFY availableViewModeListChanged)
    Q_PROPERTY(int currentZoom READ currentZoom NOTIFY currentZoomChanged)

public:
    explicit NotationStatusBarModel(QObject* parent = nullptr);

    QString accessibilityInfo() const;
    QString currentWorkspaceName() const;
    bool concertPitchEnabled() const;
    QVariant currentViewMode() const;
    QVariantList availableViewModeList() const;
    int currentZoom() const;

    Q_INVOKABLE void load();

    Q_INVOKABLE void selectWorkspace();
    Q_INVOKABLE void toggleConcertPitch();
    Q_INVOKABLE void setCurrentViewMode(const QString& modeCode);
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();

signals:
    void accessibilityInfoChanged();
    void currentWorkspaceNameChanged();
    void concertPitchEnabledChanged();
    void currentViewModeChanged();
    void availableViewModeListChanged();
    void currentZoomChanged();

private:
    notation::INotationPtr notation() const;
    notation::INotationStylePtr style() const;
    notation::INotationAccessibilityPtr accessibility() const;

    void listenChangesInAccessibility();
    void listenChangesInStyle();

    void setConcertPitchEnabled(bool enabled);
};
}

#endif // MU_NOTATION_NOTATIONSTATUSBARMODEL_H
