/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include "View.h"
#include "core/views/ClassicIndicatorWindowViewInterface.h"

namespace KDDockWidgets {

namespace Core {
class ClassicDropIndicatorOverlay;
class Group;
}

namespace flutter {

class IndicatorWindow : public flutter::View, public Core::ClassicIndicatorWindowViewInterface
{
public:
    /// When we have proper multi-window support in flutter, we can remove the parent
    explicit IndicatorWindow(Core::ClassicDropIndicatorOverlay *, Core::View *parent);
    ~IndicatorWindow() override;

    DropLocation hover(Point globalPos) override;
    void updatePositions() override;
    Point posForIndicator(DropLocation) const override;
    void raise() override;
    void setVisible(bool) override;
    bool isWindow() const override;
    void setGeometry(Rect) override;
    void resize(Size) override;
    void setObjectName(const QString &) override;

    /// implemented in dart. workaround for multi-inheritance binding limitations
    virtual Point posForIndicator_flutter(DropLocation) const;
    virtual DropLocation hover_flutter(Point globalPos);
    virtual bool updatePositions_flutter(int overlayWidth, int overlayHeight,
                                         Core::Group *hoveredGroup, int visibleLocations);

    Core::View *rubberBand() const;
    Core::Group *hoveredGroup() const;

private:
    int visibleDropIndicatorLocations() const;
    Core::ClassicDropIndicatorOverlay *const classicIndicators;
    bool m_updatePending = false;
};

}

}
