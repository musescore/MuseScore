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

#include "dockpageview.h"

#include <QTimer>

#include "docktoolbarview.h"
#include "dockingholderview.h"
#include "dockcentralview.h"
#include "dockpanelview.h"
#include "dockstatusbarview.h"

#include "ui/view/navigationcontrol.h"

#include "log.h"

using namespace muse::dock;
using namespace muse::ui;

DockPageView::DockPageView(QQuickItem* parent)
    : QQuickItem(parent), muse::Injectable(muse::iocCtxForQmlObject(this)),
    m_mainToolBars(this),
    m_toolBars(this),
    m_toolBarsDockingHolders(this),
    m_panels(this),
    m_panelsDockingHolders(this)
{
    //! NOTE: dockwindow controls the visible state
    setVisible(false);
}

void DockPageView::init()
{
    TRACEFUNC;

    for (DockBase* dock : allDocks()) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
        dock->setParentItem(this);
#endif
        dock->init();

        connect(dock, &DockBase::floatingChanged, [this](){
            reorderSections();
        });

        connect(dock, &DockBase::visibleChanged, [this](){
            reorderSections();
        });
    }

    emit inited();
}

void DockPageView::deinit()
{
    TRACEFUNC;

    for (DockBase* dock : allDocks()) {
        dock->disconnect(this);
    }
}

QString DockPageView::uri() const
{
    return m_uri;
}

void DockPageView::setParams(const QVariantMap& params)
{
    emit setParamsRequested(params);
}

QQmlListProperty<DockToolBarView> DockPageView::mainToolBarsProperty()
{
    return m_mainToolBars.property();
}

QQmlListProperty<DockToolBarView> DockPageView::toolBarsProperty()
{
    return m_toolBars.property();
}

QQmlListProperty<DockPanelView> DockPageView::panelsProperty()
{
    return m_panels.property();
}

QQmlListProperty<DockingHolderView> DockPageView::toolBarsDockingHoldersProperty()
{
    return m_toolBarsDockingHolders.property();
}

QQmlListProperty<DockingHolderView> DockPageView::panelsDockingHoldersProperty()
{
    return m_panelsDockingHolders.property();
}

QList<DockToolBarView*> DockPageView::mainToolBars() const
{
    return m_mainToolBars.list();
}

QList<DockToolBarView*> DockPageView::toolBars() const
{
    return m_toolBars.list();
}

QList<DockingHolderView*> DockPageView::toolBarsHolders() const
{
    return m_toolBarsDockingHolders.list();
}

DockCentralView* DockPageView::centralDock() const
{
    return m_central;
}

DockStatusBarView* DockPageView::statusBar() const
{
    return m_statusBar;
}

QList<DockPanelView*> DockPageView::panels() const
{
    return m_panels.list();
}

QList<DockingHolderView*> DockPageView::panelsHolders() const
{
    return m_panelsDockingHolders.list();
}

DockBase* DockPageView::dockByName(const QString& dockName) const
{
    for (DockBase* dock : allDocks()) {
        if (dock->objectName() == dockName) {
            return dock;
        }
    }

    return nullptr;
}

DockingHolderView* DockPageView::holder(DockType type, Location location) const
{
    QList<DockingHolderView*> holders;

    if (type == DockType::ToolBar) {
        holders = m_toolBarsDockingHolders.list();
    } else if (type == DockType::Panel) {
        holders = m_panelsDockingHolders.list();
    }

    for (DockingHolderView* holder : holders) {
        if (holder->location() == location) {
            return holder;
        }
    }

    return nullptr;
}

QList<DockPanelView*> DockPageView::findPanelsForDropping(const DockPanelView* panel) const
{
    QList<DockPanelView*> result;

    for (DockPanelView* destinationPanel : panels()) {
        if (destinationPanel->isTabAllowed(panel)) {
            result << destinationPanel;
        }
    }

    return result;
}

DockPanelView* DockPageView::findPanelForTab(const DockPanelView* tab) const
{
    for (DockPanelView* destinationPanel: panels()) {
        if (destinationPanel->isTabAllowed(tab)
            && destinationPanel->location() == tab->location()) {
            return destinationPanel;
        }
    }

    return nullptr;
}

bool DockPageView::isDockOpenAndCurrentInFrame(const QString& dockName) const
{
    const DockBase* dock = dockByName(dockName);
    if (!dock) {
        return false;
    }

    const bool isDockOpen = dock && dock->isOpen();

    const DockPanelView* panel = dynamic_cast<const DockPanelView*>(dock);
    if (panel) {
        return isDockOpen && panel->isCurrentTabInFrame();
    }

    return isDockOpen;
}

void DockPageView::toggleDock(const QString& dockName)
{
    setDockOpen(dockName, !isDockOpenAndCurrentInFrame(dockName));
}

void DockPageView::setDockOpen(const QString& dockName, bool open)
{
    DockBase* dock = dockByName(dockName);
    if (!dock) {
        return;
    }

    if (!open) {
        dock->close();
        return;
    }

    DockPanelView* panel = dynamic_cast<DockPanelView*>(dock);
    if (!panel) {
        dock->open();
        return;
    }

    DockPanelView* destinationPanel = findPanelForTab(panel);
    if (destinationPanel) {
        destinationPanel->addPanelAsTab(panel);
        panel->makeCurrentTabInFrame();
    } else {
        panel->open();
    }
}

void DockPageView::reorderSections()
{
    //! NOTE: In some cases, such as setting visible true,
    //! it is necessary to give the UI time to render the content, so we will add a delay.
    QTimer::singleShot(2000, this, [this](){
        doReorderSections();
    });
}

void DockPageView::doReorderSections()
{
    TRACEFUNC;

    if (!isVisible()) {
        return;
    }

    QList<DockBase*> docks = allDocks();
    QList<DockBase*> docksAvailableForNavigation;

    for (DockBase* dock: docks) {
        if (dock->navigationSection() && dock->isVisible()) {
            docksAvailableForNavigation.append(dock);
        }
    }

    reorderDocksNavigationSections(docksAvailableForNavigation);
}

void DockPageView::reorderDocksNavigationSections(QList<DockBase*>& docks)
{
    QHash<muse::ui::INavigationSection*, QList<DockBase*> > orderedSections;
    for (DockBase* dock: docks) {
        if (ui::INavigationSection* section = dock->navigationSection()) {
            orderedSections[section] << dock;
        }
    }

    for (QList<DockBase*>& panels : orderedSections.values()) {
        reorderNavigationSectionPanels(panels);
    }
}

void DockPageView::reorderNavigationSectionPanels(QList<DockBase*>& sectionDocks)
{
    std::sort(sectionDocks.begin(), sectionDocks.end(), [](DockBase* dock1, DockBase* dock2) {
        if (!dock1->navigationSection() || !dock2->navigationSection()) {
            return false;
        }

        if (dock1 == dock2) {
            return false;
        }

        QPoint dock1Pos = dock1->globalPosition();
        QPoint dock2Pos = dock2->globalPosition();

        if (dock1->floating() && dock2->floating()) {
            return dock1Pos.x() < dock2Pos.x();
        } else if (dock1->floating()) {
            return false;
        } else if (dock2->floating()) {
            return true;
        }

        if (dock1Pos.x() == dock2Pos.x()) {
            return dock1Pos.y() < dock2Pos.y();
        }

        return dock1Pos.x() < dock2Pos.x();
    });

    //!NOTE: It is possible that the dock does not contain all the panels.
    //! For example, MainToolBar is created for the entire window and is always visible.
    //! Reserve n panels for each dock.
    int i = 10;
    QHash<DockBase*, int> orderedDocks;
    for (DockBase* dock: sectionDocks) {
        //!NOTE: It is possible that the dock contains multiple panels.
        //! Reserve n panels for each dock.
        int order = i++ *1000;

        //! NOTE: If a panel is inside a frame with another panel,
        //! there is no need to set the order for the frame panel, as it is already set.
        bool exists = false;
        for (DockBase* orderedPanel: orderedDocks.keys()) {
            if (orderedPanel->isInSameFrame(dock)) {
                exists = true;
            }
        }

        if (!exists) {
            orderedDocks[dock] = order;
            dock->setFramePanelOrder(order);
        }

        dock->setContentNavigationPanelOrderStart(order + 1);
    }
}

bool DockPageView::isDockFloating(const QString& dockName) const
{
    const DockBase* dock = dockByName(dockName);
    return dock ? dock->floating() : false;
}

void DockPageView::toggleDockFloating(const QString& dockName)
{
    DockBase* dock = dockByName(dockName);
    if (!dock) {
        return;
    }

    dock->setFloating(!dock->floating());
}

void DockPageView::setUri(const QString& uri)
{
    if (uri == m_uri) {
        return;
    }

    m_uri = uri;
    emit uriChanged(uri);
}

void DockPageView::setCentralDock(DockCentralView* central)
{
    if (central == m_central) {
        return;
    }

    m_central = central;
    emit centralDockChanged(central);
}

void DockPageView::setStatusBar(DockStatusBarView* statusBar)
{
    if (statusBar == m_statusBar) {
        return;
    }

    m_statusBar = statusBar;
    emit statusBarChanged(statusBar);
}

void DockPageView::componentComplete()
{
    QQuickItem::componentComplete();

    Q_ASSERT(!m_uri.isEmpty());
    Q_ASSERT(m_central != nullptr);

    connect(this, &DockPageView::visibleChanged, [this](){
        if (isVisible()) {
            reorderSections();
        }
    });
}

QList<DockBase*> DockPageView::allDocks() const
{
    auto mainToolBars = this->mainToolBars();
    auto toolbars = this->toolBars();
    auto panels = this->panels();

    QList<DockBase*> docks;
    docks << QList<DockBase*>(mainToolBars.begin(), mainToolBars.end());
    docks << QList<DockBase*>(toolbars.begin(), toolbars.end());
    docks << QList<DockBase*>(panels.begin(), panels.end());

    docks << m_central;

    if (m_statusBar) {
        docks << m_statusBar;
    }

    return docks;
}

void DockPageView::setDefaultNavigationControl(muse::ui::NavigationControl* control)
{
    muse::ui::INavigationControl* _control = dynamic_cast<muse::ui::INavigationControl*>(control);
    navigationController()->setDefaultNavigationControl(_control);
}

void DockPageView::forceLayout()
{
    emit layoutRequested();
}

QVariant DockPageView::tours() const
{
    return m_tours;
}

void DockPageView::setTours(const QVariant& newTours)
{
    if (m_tours == newTours) {
        return;
    }

    for (const QVariant& tourVar: newTours.toList()) {
        QVariantMap tourMap = tourVar.toMap();

        String eventCode = tourMap.value("eventCode").toString();

        QVariantMap tourInfoMap = tourMap.value("tour").toMap();

        tours::Tour tour;

        tour.id = tourInfoMap.value("id").toString();

        for (const QVariant& stepVar: tourInfoMap.value("steps").toList()) {
            QVariantMap stepMap = stepVar.toMap();

            tours::TourStep step;
            step.title = stepMap.value("title").toString();
            step.description = stepMap.value("description").toString();
            step.previewImageOrGifUrl = stepMap.value("previewImageOrGifUrl").toString();
            step.videoExplanationUrl = stepMap.value("videoExplanationUrl").toString();
            step.controlUri = Uri(stepMap.value("controlUri").toString());

            tour.steps.emplace_back(step);
        }

        if (!tour.steps.empty()) {
            toursService()->registerTour(eventCode, tour);
        }
    }

    m_tours = newTours;
    emit toursChanged();
}
