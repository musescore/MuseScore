/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "View.h"
#include "core/Utils_p.h"
#include "core/View_p.h"
#include "kddockwidgets/core/Controller.h"
#include "core/EventFilterInterface.h"

#ifdef KDDW_FRONTEND_QTWIDGETS
#include <QWidget>
#endif

#ifdef KDDW_FRONTEND_QTQUICK
#include <QQuickItem>
#endif

using namespace KDDockWidgets::QtCommon;

// clazy:excludeall=missing-qobject-macro


#if defined(DOCKS_DEVELOPER_MODE)
int View_qt::s_logicalDpiFactorOverride = 0;
#endif

class View_qt::EventFilter : public QObject
{
public:
    explicit EventFilter(View_qt *view, QObject *target)
        : q(view)
    {
        target->installEventFilter(this);
    }

    ~EventFilter() override;

    bool eventFilter(QObject *, QEvent *ev) override
    {
        return q->deliverViewEventToFilters(ev);
    }

    View_qt *const q;
};

View_qt::View_qt(Core::Controller *controller, Core::ViewType type, QObject *thisObj)
    : View(controller, type)
    , m_eventFilter(thisObj ? new EventFilter(this, thisObj) : nullptr)
    , m_thisObj(thisObj)
{
}

View_qt::~View_qt()
{
    delete m_eventFilter;
}

View_qt::EventFilter::~EventFilter() = default;

QObject *View_qt::thisObject() const
{
    return m_thisObj;
}

KDDockWidgets::Core::HANDLE View_qt::handle() const
{
    return m_thisObj;
}

void View_qt::setViewName(const QString &name)
{
    if (m_thisObj) {
        m_thisObj->setObjectName(name);
    }
}

/*static*/
QObject *View_qt::asQObject(View *view)
{
    if (auto viewqt = dynamic_cast<View_qt *>(view))
        return viewqt->thisObject();

    return nullptr;
}

#ifdef KDDW_FRONTEND_QTWIDGETS

/*static */
QWidget *View_qt::asQWidget(View *view)
{
    return qobject_cast<QWidget *>(asQObject(view));
}

/*static */
QWidget *View_qt::asQWidget(Core::Controller *controller)
{
    if (!controller)
        return nullptr;

    return asQWidget(controller->view());
}

#endif

#ifdef KDDW_FRONTEND_QTQUICK

/*static */
QQuickItem *View_qt::asQQuickItem(View *view)
{
    return qobject_cast<QQuickItem *>(asQObject(view));
}

#endif
