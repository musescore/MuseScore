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

#include "docksetup.h"

#include "internal/dropindicators.h"
#include "internal/dockseparator.h"
#include "internal/dockframemodel.h"
#include "internal/dockwindowactionscontroller.h"

#include "dockwindow.h"
#include "dockpanel.h"
#include "dockpanelholder.h"
#include "dockstatusbar.h"
#include "docktoolbarholder.h"
#include "dockcentral.h"
#include "dockpage.h"

#include "docktypes.h"

#include "thirdparty/KDDockWidgets/src/Config.h"
#include "thirdparty/KDDockWidgets/src/DockWidgetBase.h"
#include "thirdparty/KDDockWidgets/src/FrameworkWidgetFactory.h"
#include "thirdparty/KDDockWidgets/src/private/FloatingWindow_p.h"

#include <QQmlEngine>

namespace mu::dock {
class DockWidgetFactory : public KDDockWidgets::DefaultWidgetFactory
{
public:
    KDDockWidgets::DropIndicatorOverlayInterface* createDropIndicatorOverlay(KDDockWidgets::DropArea* dropArea) const override
    {
        return new DropIndicators(dropArea);
    }

    Layouting::Separator* createSeparator(Layouting::Widget* parent = nullptr) const override
    {
        return new DockSeparator(parent);
    }

    QUrl titleBarFilename() const override
    {
        return QUrl("qrc:/qml/dockwindow/DockTitleBar.qml");
    }

    QUrl frameFilename() const override
    {
        return QUrl("qrc:/qml/dockwindow/DockFrame.qml");
    }

    QUrl floatingWindowFilename() const override
    {
        return QUrl("qrc:/qml/dockwindow/DockFloatingWindow.qml");
    }
};
}

using namespace mu::dock;

static std::shared_ptr<DockWindowActionsController> s_actionsController = std::make_shared<DockWindowActionsController>();

void DockSetup::registerQmlTypes()
{
    qmlRegisterType<DockWindow>("MuseScore.Dock", 1, 0, "DockWindow");
    qmlRegisterType<DockPanel>("MuseScore.Dock", 1, 0, "DockPanel");
    qmlRegisterType<DockPanelHolder>("MuseScore.Dock", 1, 0, "DockPanelHolder");
    qmlRegisterType<DockStatusBar>("MuseScore.Dock", 1, 0, "DockStatusBar");
    qmlRegisterType<DockToolBar>("MuseScore.Dock", 1, 0, "DockToolBar");
    qmlRegisterType<DockToolBarHolder>("MuseScore.Dock", 1, 0, "DockToolBarHolder");
    qmlRegisterType<DockCentral>("MuseScore.Dock", 1, 0, "DockCentral");
    qmlRegisterType<DockPage>("MuseScore.Dock", 1, 0, "DockPage");
    qmlRegisterType<DockFrameModel>("MuseScore.Dock", 1, 0, "DockFrameModel");
    qmlRegisterType<DockBase>("MuseScore.Dock", 1, 0, "DockBase");

    qRegisterMetaType<DropIndicators*>();
}

void DockSetup::setup(QQmlEngine* engine)
{
    KDDockWidgets::Config::self().setFrameworkWidgetFactory(new DockWidgetFactory());
    KDDockWidgets::Config::self().setQmlEngine(engine);

    auto flags = KDDockWidgets::Config::self().flags()
                 | KDDockWidgets::Config::Flag_HideTitleBarWhenTabsVisible;

    KDDockWidgets::Config::self().setFlags(flags);

    KDDockWidgets::FloatingWindow::s_windowFlagsOverride = Qt::Tool
                                                           | Qt::NoDropShadowWindowHint
                                                           | Qt::FramelessWindowHint;

    auto internalFlags = KDDockWidgets::Config::self().internalFlags()
                         | KDDockWidgets::Config::InternalFlag_UseTransparentFloatingWindow;

    KDDockWidgets::Config::self().setInternalFlags(internalFlags);

    QSize minDockSize = KDDockWidgets::Config::self().absoluteWidgetMinSize();
    minDockSize.setHeight(30);
    KDDockWidgets::Config::self().setAbsoluteWidgetMinSize(minDockSize);

    KDDockWidgets::Config::self().setSeparatorThickness(1);
}

void DockSetup::onInit()
{
    s_actionsController->init();
}
