/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "Separator.h"
#include "layouting/Item_p.h"
#include "layouting/LayoutingSeparator_p.h"
#include "View.h"
#include "Logging_p.h"
#include "Layout.h"
#include "Config.h"
#include "Platform.h"
#include "Controller.h"
#include "core/ViewFactory.h"


#ifdef Q_OS_WIN
#include <Windows.h>
#endif

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

/// @brief internal counter just for unit-tests
static int s_numSeparators = 0;

namespace {

bool rubberBandIsTopLevel()
{
    return KDDockWidgets::Config::self().internalFlags()
        & KDDockWidgets::Config::InternalFlag_TopLevelIndicatorRubberBand;
}

}

struct Separator::Private : public LayoutingSeparator
{
    // Only set when anchor is moved through mouse. Side1 if going towards left or top, Side2
    // otherwise.

    explicit Private(Core::Separator *qq, LayoutingHost *host, Qt::Orientation orientation, Core::ItemBoxContainer *parentContainer)
        : LayoutingSeparator(host, orientation, parentContainer)
        , q(qq)
    {
        s_numSeparators++;
    }

    ~Private() override;

    Rect geometry() const override
    {
        return m_geometry;
    }

    void setGeometry(Rect r) override
    {
        q->setGeometry(r);
    }

    void free() override
    {
#ifdef KDDW_FRONTEND_QT
        if (Config::self().internalFlags() & Config::InternalFlag_DeleteSeparatorsLater) {
            q->deleteLater();
            return;
        }
#endif
        delete q;
    }

    void raise() override
    {
        q->view()->raise();
    }

    Core::Separator *const q;
    Rect m_geometry;
    int lazyPosition = 0;
    View *lazyResizeRubberBand = nullptr;
    const bool usesLazyResize = Config::self().flags() & Config::Flag_LazyResize;
};

namespace {
Core::View *viewForLayoutingHost(LayoutingHost *host)
{
    // For KDDW, a LayoutingHost is always a Core::Layout
    if (auto layout = Layout::fromLayoutingHost(host))
        return layout->view();

    return nullptr;
}
}

Separator::Separator(LayoutingHost *host, Qt::Orientation orientation, Core::ItemBoxContainer *parentContainer)
    : Controller(ViewType::Separator, Config::self().viewFactory()->createSeparator(this, viewForLayoutingHost(host)))
    , d(new Private(this, host, orientation, parentContainer))
{
    view()->show();
    view()->init();
    d->lazyResizeRubberBand = d->usesLazyResize ? Config::self().viewFactory()->createRubberBand(
                                                      rubberBandIsTopLevel() ? nullptr : view())
                                                : nullptr;
    setVisible(true);
}

Separator::~Separator()
{
    delete d;
}

bool Separator::isVertical() const
{
    return d->isVertical();
}

int Separator::position() const
{
    return d->position();
}

void Separator::setGeometry(Rect r)
{
    if (r == d->m_geometry)
        return;

    d->m_geometry = r;

    if (View *v = view()) {
        v->setGeometry(r);
    }

    setVisible(true);
}

void Separator::setLazyPosition(int pos)
{
    if (pos == d->lazyPosition)
        return;

    View *v = view();

    d->lazyPosition = pos;

    Rect geo = v->geometry();
    if (isVertical()) {
        geo.moveTop(pos);
    } else {
        geo.moveLeft(pos);
    }

    if (rubberBandIsTopLevel() && Platform::instance()->isQtWidgets())
        geo.translate(view()->mapToGlobal(Point(0, 0)));
    d->lazyResizeRubberBand->setGeometry(geo);
}

bool Separator::usesLazyResize() const
{
    return d->usesLazyResize;
}

void Separator::onMousePress()
{
    d->onMousePress();

    KDDW_DEBUG("Drag started");

    if (d->lazyResizeRubberBand) {
        setLazyPosition(position());
        d->lazyResizeRubberBand->show();
        if (rubberBandIsTopLevel() && Platform::instance()->isQtWidgets())
            d->lazyResizeRubberBand->raise();
    }
}

void Separator::onMouseReleased()
{
    if (d->lazyResizeRubberBand) {
        d->lazyResizeRubberBand->hide();
        d->m_parentContainer->requestSeparatorMove(d, d->lazyPosition - position());
    }

    d->onMouseRelease();
}

void Separator::onMouseDoubleClick()
{
    // a double click means we'll resize the left and right neighbour so that they occupy
    // the same size (or top/bottom, depending on orientation).
    d->m_parentContainer->requestEqualSize(d);
}

void Separator::onMouseMove(Point pos)
{
    if (!d->isBeingDragged())
        return;

    if (Platform::instance()->isQt()) {
        // Workaround a bug in Qt where we're getting mouse moves without without the button being
        // pressed
        if (!Platform::instance()->isLeftMouseButtonPressed()) {
            KDDW_DEBUG(
                "Separator::onMouseMove: Ignoring spurious mouse event. Someone ate our ReleaseEvent");
            onMouseReleased();
            return;
        }

#ifdef KDDW_FRONTEND_QT_WINDOWS
        // Try harder, Qt can be wrong, if mixed with MFC
        const bool mouseButtonIsReallyDown =
            (GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000);
        if (!mouseButtonIsReallyDown) {
            KDDW_DEBUG(
                "Separator::onMouseMove: Ignoring spurious mouse event. Someone ate our ReleaseEvent");
            onMouseReleased();
            return;
        }
#endif
    }

    if (d->lazyResizeRubberBand) {
        const int positionToGoTo = d->onMouseMove(pos, /*moveSeparator=*/false);
        if (positionToGoTo != -1)
            setLazyPosition(positionToGoTo);
    } else {
        d->onMouseMove(pos, /*moveSeparator=*/true);
    }
}

LayoutingSeparator *Separator::asLayoutingSeparator() const
{
    return d;
}

/** static */
bool Separator::isResizing()
{
    return LayoutingSeparator::s_separatorBeingDragged != nullptr;
}

/** static */
int Separator::numSeparators()
{
    return s_numSeparators;
}

Separator::Private::~Private()
{
    s_numSeparators--;
}
