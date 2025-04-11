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

#ifndef MU_NOTATION_NOTATIONSTATUSBARMODEL_H
#define MU_NOTATION_NOTATIONSTATUSBARMODEL_H

#include <QObject>
#include <QQmlParserStatus>

#include "async/asyncable.h"
#include "actions/actionable.h"

#include "uicomponents/view/menuitem.h"
#include "uicomponents/view/abstractmenumodel.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "ui/iuiactionsregister.h"
#include "workspace/iworkspaceconfiguration.h"
#include "notation/inotationconfiguration.h"
#include "context/iglobalcontext.h"

#include "notation/notationtypes.h"

#include "global/iglobalconfiguration.h"

namespace mu::appshell {
class NotationStatusBarModel : public QObject, public QQmlParserStatus, public muse::Injectable, public muse::async::Asyncable,
    public muse::actions::Actionable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString accessibilityInfo READ accessibilityInfo NOTIFY accessibilityInfoChanged)
    Q_PROPERTY(QVariant currentWorkspaceItem READ currentWorkspaceItem CONSTANT)
    Q_PROPERTY(QVariant concertPitchItem READ concertPitchItem CONSTANT)
    Q_PROPERTY(QVariant currentViewMode READ currentViewMode NOTIFY currentViewModeChanged)
    Q_PROPERTY(bool zoomEnabled READ zoomEnabled NOTIFY zoomEnabledChanged)
    Q_PROPERTY(QVariantList availableViewModeList READ availableViewModeList_property NOTIFY availableViewModeListChanged)
    Q_PROPERTY(QVariantList availableZoomList READ availableZoomList_property NOTIFY availableZoomListChanged)
    Q_PROPERTY(int currentZoomPercentage READ currentZoomPercentage WRITE setCurrentZoomPercentage NOTIFY currentZoomPercentageChanged)

    muse::Inject<context::IGlobalContext> context = { this };
    muse::Inject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::Inject<muse::ui::IUiActionsRegister> actionsRegister = { this };
    muse::Inject<muse::workspace::IWorkspaceConfiguration> workspaceConfiguration = { this };
    muse::Inject<notation::INotationConfiguration> notationConfiguration = { this };
    muse::Inject<muse::IGlobalConfiguration> globalConfiguration = { this };

public:
    explicit NotationStatusBarModel(QObject* parent = nullptr);

    QString accessibilityInfo() const;
    QVariant currentWorkspaceItem();
    QVariant concertPitchItem();
    QVariant currentViewMode();
    bool zoomEnabled() const;
    int currentZoomPercentage() const;

    Q_INVOKABLE void toggleConcertPitch();
    Q_INVOKABLE void setCurrentViewMode(const QString& modeCode);

    Q_INVOKABLE int minZoomPercentage() const;
    Q_INVOKABLE int maxZoomPercentage() const;
    Q_INVOKABLE void setCurrentZoom(const QString& zoomId);
    Q_INVOKABLE void zoomIn();
    Q_INVOKABLE void zoomOut();

    Q_INVOKABLE void handleAction(const QString& actionCode);
    Q_INVOKABLE void handleWorkspacesMenuItem(const QString& itemId);

public slots:
    void setCurrentZoomPercentage(int zoomPercentage);

signals:
    void accessibilityInfoChanged();
    void currentWorkspaceActionChanged();
    void concertPitchActionChanged();
    void currentViewModeChanged();
    void availableViewModeListChanged();
    void zoomEnabledChanged();
    void availableZoomListChanged();
    void currentZoomPercentageChanged();

private:
    void classBegin() override;
    void componentComplete() override {}

    notation::INotationPtr notation() const;
    notation::INotationAccessibilityPtr accessibility() const;

    void initAvailableViewModeList();
    void initAvailableZoomList();

    muse::uicomponents::MenuItem* makeMenuItem(const muse::actions::ActionCode& actionCode);

    void updateConcertPitchItem();
    void updateCurrentWorkspaceItem();

    void dispatch(const muse::actions::ActionCode& code, const muse::actions::ActionData& args = muse::actions::ActionData());

    void onCurrentNotationChanged();

    notation::ZoomType currentZoomType() const;

    void listenChangesInAccessibility();

    QList<int> possibleZoomPercentageList() const;

    QVariantList availableViewModeList_property();
    QVariantList availableZoomList_property();

    QVariantList menuItemListToVariantList(const muse::uicomponents::MenuItemList& list) const;

    muse::uicomponents::MenuItem* m_currentWorkspaceItem = nullptr;
    muse::uicomponents::MenuItem* m_concertPitchItem = nullptr;
    muse::uicomponents::MenuItemList m_availableViewModeList;
    muse::uicomponents::MenuItemList m_availableZoomList;

    std::shared_ptr<muse::uicomponents::AbstractMenuModel> m_workspacesMenuModel;
};
}

#endif // MU_NOTATION_NOTATIONSTATUSBARMODEL_H
