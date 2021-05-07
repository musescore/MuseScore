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
#include "ui/imainwindow.h"

#include "modularity/ioc.h"
#include "ui/imainwindow.h"

#include "thirdparty/KDDockWidgets/src/private/DropIndicatorOverlayInterface_p.h"

namespace mu::dock {
class IndicatorsWindow;
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

    INJECT(dock, ui::IMainWindow, mainWindow)

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
    friend class IndicatorsWindow;

    bool isIndicatorVisible(DropLocation location) const;
    bool isDropAllowed(DropLocation location) const;

    bool isToolBar() const;

    bool hoveringOverDock(DockType type) const;

    const KDDockWidgets::DockWidgetBase* hoveredDock() const;
    const KDDockWidgets::DockWidgetBase* draggedDock() const;

    void setDropLocation(DropLocation);
    void updateWindowPosition();

    bool onResize(QSize newSize) override;
    void updateVisibility() override;

    void showDropAreaIfNeed(const QPoint& hoveredGlobalPos);

    KDDockWidgets::QWidgetOrQuick* m_rubberBand = nullptr;
    IndicatorsWindow* m_indicatorsWindow = nullptr;
    DockProperties m_draggedDockProperties;
};
}

#endif // MU_DOCK_DROPINDICATORS_H
