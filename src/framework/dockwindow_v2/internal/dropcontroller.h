/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#ifndef MUSE_DOCK_DROPCONTROLLER_H
#define MUSE_DOCK_DROPCONTROLLER_H

#include "modularity/ioc.h"
#include "../idockwindowprovider.h"

#include "dockbase.h"

#include "kddockwidgets/src/core/views/ClassicIndicatorWindowViewInterface.h"
#include "kddockwidgets/src/core/indicators/ClassicDropIndicatorOverlay.h"

namespace muse::dock {
class DockPanelView;
class DockingHolderView;
class DockToolBarView;
class DockPageView;

class DropController : public KDDockWidgets::Core::ClassicIndicatorWindowViewInterface, public Contextable
{
    ContextInject<IDockWindowProvider> dockWindowProvider = { this };

public:
    explicit DropController(KDDockWidgets::Core::ClassicDropIndicatorOverlay* classicIndicators, KDDockWidgets::Core::View* parent,
                            const modularity::ContextPtr& iocCtx);

    // ClassicIndicatorWindowViewInterface
    void setObjectName(const QString&) override {}
    KDDockWidgets::DropLocation hover(KDDockWidgets::Point globalPos) override;
    KDDockWidgets::Point posForIndicator(KDDockWidgets::DropLocation) const override;
    void updatePositions() override {}
    void raise() override {}
    void setVisible(bool visible) override;
    void resize(KDDockWidgets::Size) override {}
    void setGeometry(KDDockWidgets::Rect) override {}
    bool isWindow() const override { return false; }
    void updateIndicatorVisibility() override {}

private:
    void endHover();

    bool isMouseOverDock(const QPoint& mouseLocalPos, const DockBase* dock) const;
    void updateToolBarOrientation(DockToolBarView* draggedToolBar, const DropDestination& dropDestination = DropDestination());
    void setCurrentDropDestination(const DockBase* draggedDock, const DropDestination& dropDestination);

    DropDestination resolveDropDestination(const DockBase* draggedDock, const QPoint& localPos) const;
    DockingHolderView* resolveDockingHolder(DockType draggedDockType, const QPoint& localPos) const;
    DockPanelView* resolvePanelForDrop(const DockPanelView* panel, const QPoint& localPos) const;
    Location resolveDropLocation(const DockBase* hoveredDock, const QPoint& localPos) const;
    QRect resolveHighlightingRect(const DockBase* draggedDock, const DropDestination& destination) const;

    IDockWindow* dockWindow() const;
    DockPageView* currentPage() const;
    DockBase* draggedDock() const;

    KDDockWidgets::Core::ClassicDropIndicatorOverlay* m_classicIndicators = nullptr;
    DropDestination m_currentDropDestination;
};
}

#endif // MUSE_DOCK_DROPCONTROLLER_H
