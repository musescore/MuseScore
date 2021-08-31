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

#ifndef MU_DOCK_DROPINDICATORS_H
#define MU_DOCK_DROPINDICATORS_H

#include "modularity/ioc.h"
#include "../docktypes.h"
#include "../idockwindowprovider.h"
#include "globaltypes.h"

#include "thirdparty/KDDockWidgets/src/private/DropIndicatorOverlayInterface_p.h"

namespace mu::dock {
class DropIndicatorsWindow;
class DropIndicators : public KDDockWidgets::DropIndicatorOverlayInterface
{
    Q_OBJECT

    Q_PROPERTY(bool outterLeftIndicatorVisible READ outterLeftIndicatorVisible NOTIFY indicatorsVisibilityChanged)
    Q_PROPERTY(bool outterRightIndicatorVisible READ outterRightIndicatorVisible NOTIFY indicatorsVisibilityChanged)
    Q_PROPERTY(bool outterTopIndicatorVisible READ outterTopIndicatorVisible NOTIFY indicatorsVisibilityChanged)
    Q_PROPERTY(bool outterBottomIndicatorVisible READ outterBottomIndicatorVisible NOTIFY indicatorsVisibilityChanged)

    Q_PROPERTY(bool centralIndicatorVisible READ centralIndicatorVisible NOTIFY indicatorsVisibilityChanged)

    Q_PROPERTY(bool innerLeftIndicatorVisible READ innerLeftIndicatorVisible NOTIFY indicatorsVisibilityChanged)
    Q_PROPERTY(bool innerRightIndicatorVisible READ innerRightIndicatorVisible NOTIFY indicatorsVisibilityChanged)
    Q_PROPERTY(bool innerTopIndicatorVisible READ innerTopIndicatorVisible NOTIFY indicatorsVisibilityChanged)
    Q_PROPERTY(bool innerBottomIndicatorVisible READ innerBottomIndicatorVisible NOTIFY indicatorsVisibilityChanged)

    INJECT(dock, IDockWindowProvider, dockWindowProvider)

public:
    explicit DropIndicators(KDDockWidgets::DropArea* dropArea);
    ~DropIndicators() override;

    DropLocation hover_impl(QPoint globalPos) override;
    QPoint posForIndicator(DropLocation) const override;

    bool outterLeftIndicatorVisible() const;
    bool outterRightIndicatorVisible() const;
    bool outterTopIndicatorVisible() const;
    bool outterBottomIndicatorVisible() const;

    bool centralIndicatorVisible() const;

    bool innerLeftIndicatorVisible() const;
    bool innerRightIndicatorVisible() const;
    bool innerTopIndicatorVisible() const;
    bool innerBottomIndicatorVisible() const;

signals:
    void indicatorsVisibilityChanged();

private:
    bool onResize(QSize newSize) override;
    void updateVisibility() override;

    bool isIndicatorVisible(DropLocation location) const;
    bool isDropAllowed(DropLocation location) const;
    bool isDropOnHoveredDockAllowed() const;
    bool isDraggedDockToolBar() const;
    bool isDraggedDockPanel() const;
    bool needShowToolBarHolders() const;
    bool needShowPanelHolders() const;

    const KDDockWidgets::DockWidgetBase* hoveredDock() const;
    const KDDockWidgets::DockWidgetBase* draggedDock() const;

    DockType dockType(const KDDockWidgets::DockWidgetBase* dock) const;
    framework::Orientation dockOrientation(const KDDockWidgets::DockWidgetBase& dock) const;

    DropLocation dropLocationForToolBar(const QPoint& hoveredGlobalPos) const;
    QRect dropAreaRectForToolBar(DropLocation location) const;
    QRect dropAreaRectForPanel(DropLocation location) const;

    void showDropAreaIfNeed(const QRect& dropRect);
    void hideDropArea();
    void updateToolBarOrientation();

    void updateWindowPosition();

    IDockWindow* dockWindow() const;

    KDDockWidgets::QWidgetOrQuick* m_rubberBand = nullptr;
    DropIndicatorsWindow* m_indicatorsWindow = nullptr;
};
}

#endif // MU_DOCK_DROPINDICATORS_H
