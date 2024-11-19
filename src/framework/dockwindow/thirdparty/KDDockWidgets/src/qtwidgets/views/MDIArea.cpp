/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "MDIArea.h"

#include "core/View_p.h"

#include "qtwidgets/ViewFactory.h"
#include "kddockwidgets/core/views/DockWidgetViewInterface.h"
#include "kddockwidgets/core/DockWidget.h"
#include "kddockwidgets/core/MDILayout.h"
#include "kddockwidgets/core/DropArea.h"

#include "qtwidgets/views/View.h"
#include "Config.h"

#include <QVBoxLayout>

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;
using namespace KDDockWidgets::QtWidgets;

class MDIArea::Private
{
public:
    explicit Private(View *parent)
        : layout(new MDILayout(parent))
    {
    }

    ~Private()
    {
        delete layout;
    }

    MDILayout *const layout;
};

MDIArea::MDIArea(QWidget *parent)
    : QtWidgets::View<QWidget>(nullptr, ViewType::None, parent)
    , d(new Private(this))
{

    auto vlay = new QVBoxLayout(this);
    vlay->addWidget(View_qt::asQWidget(d->layout));

    View::d->closeRequested.connect([this](QCloseEvent *ev) { d->layout->onCloseEvent(ev); });
}

MDIArea::~MDIArea()
{
    delete d;
}

void MDIArea::addDockWidget(Core::DockWidget *dw, QPoint localPt,
                            const InitialOption &addingOption)
{
    if (!dw)
        return;

    if (dw->options() & DockWidgetOption_MDINestable) {
        // We' wrap it with a drop area, so we can drag other dock widgets over this one and dock
        auto wrapperDW =
            Config::self()
                .viewFactory()
                ->createDockWidget(QStringLiteral("%1-mdiWrapper").arg(dw->uniqueName()))
                ->asDockWidgetController();

        auto dropAreaWrapper = new DropArea(wrapperDW->view(), {}, /*isMDIWrapper= */ true);
        dropAreaWrapper->addDockWidget(dw, Location_OnBottom, nullptr);
        wrapperDW->setGuestView(dropAreaWrapper->view()->asWrapper());

        dw = wrapperDW;
    }

    d->layout->addDockWidget(dw, localPt, addingOption);
}

void MDIArea::moveDockWidget(Core::DockWidget *dw, QPoint pos)
{
    d->layout->moveDockWidget(dw, pos);
}

void MDIArea::resizeDockWidget(Core::DockWidget *dw, QSize size)
{
    d->layout->resizeDockWidget(dw, size);
}

void MDIArea::addDockWidget(Core::DockWidgetViewInterface *dwView, QPoint localPt,
                            const InitialOption &addingOption)
{
    auto dw = dwView ? dwView->dockWidget() : nullptr;
    addDockWidget(dw, localPt, addingOption);
}

void MDIArea::moveDockWidget(Core::DockWidgetViewInterface *dwView, QPoint pos)
{
    auto dw = dwView ? dwView->dockWidget() : nullptr;
    moveDockWidget(dw, pos);
}

void MDIArea::resizeDockWidget(Core::DockWidgetViewInterface *dwView, QSize size)
{
    auto dw = dwView ? dwView->dockWidget() : nullptr;
    resizeDockWidget(dw, size);
}

QVector<Core::Group *> MDIArea::groups() const
{
    return d->layout->groups();
}
