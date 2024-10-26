/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

/**
 * @file
 * @brief Represents a dock widget.
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#include "kddockwidgets/core/views/DockWidgetViewInterface.h"
#include "View.h"

namespace KDDockWidgets {

namespace Core {
class Group;
class TitleBar;
}

namespace flutter {
/**
 * @brief Represents a dock widget.
 *
 * Most of the interface lives in Core::DockWidget, to facilitate sharing with QtQuick.
 */
class DOCKS_EXPORT DockWidget : public flutter::View,
                                public Core::DockWidgetViewInterface
{
public:
    /**
     * @brief constructs a new DockWidget
     * @param uniqueName the name of the dockwidget, should be unique. Use title for user visible
     * text.
     * @param options optional options controlling behaviour
     * @param layoutSaverOptions options regarding LayoutSaver behaviour
     * @param engine the QML engine this dock widget will be created on. If not specified then
     * Config::self().qmlEngine() will be used
     *
     * There's no parent argument. The DockWidget is either parented to FloatingWindow or MainWindow
     * when visible, or stays without a parent when hidden.
     */
    explicit DockWidget(const QString &uniqueName, DockWidgetOptions options = {},
                        LayoutSaverOptions layoutSaverOptions = {});

    ///@brief destructor
    ~DockWidget() override;

    /// @reimp
    Size minSize() const override;

    /// @reimp
    Size maxSizeHint() const override;

    Core::DockWidget *dockWidget() const;
    std::shared_ptr<Core::View> focusCandidate() const override;

    void show() override
    {
        Core::DockWidgetViewInterface::open();
    }

    void raise() override
    {
        Core::DockWidgetViewInterface::raise();
    }

protected:
    void init() override;
};

}
}
