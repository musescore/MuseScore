/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  Author: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

/**
 * @file
 * @brief FocusScope
 *
 * @author Sérgio Martins \<sergio.martins@kdab.com\>
 */

#include "FocusScope.h"
#include "TitleBar_p.h"
#include "Frame_p.h"
#include "DockWidgetBase.h"
#include "DockRegistry_p.h"

#include <QObject>
#include <QGuiApplication>
#include <QPointer>

using namespace KDDockWidgets;

// Our Private inherits from QObject since FocusScope can't (Since Frame is already QObject)
class FocusScope::Private : public QObject //clazy:exclude=missing-qobject-macro (breaks unity build with earlier cmake due to including .moc here.)
{
public:
    Private(FocusScope *qq, QWidgetAdapter *thisWidget)
        : q(qq)
        , m_thisWidget(thisWidget)
    {
        connect(qApp, &QGuiApplication::focusObjectChanged,
                this, &Private::onFocusObjectChanged);

        onFocusObjectChanged(qApp->focusObject());
        m_inCtor = false;
    }

    ~Private() override;

    void setIsFocused(bool);
    void onFocusObjectChanged(QObject *);
    bool isInFocusScope(WidgetType *) const;
    void emitDockWidgetFocusChanged();

    FocusScope *const q;
    QWidgetAdapter *const m_thisWidget;
    bool m_isFocused = false;
    bool m_inCtor = true;
    QPointer<WidgetType> m_lastFocusedInScope;
};

FocusScope::Private::~Private()
{
}

FocusScope::FocusScope(QWidgetAdapter *thisWidget)
    : d(new Private(this, thisWidget))
{
}

FocusScope::~FocusScope()
{
    delete d;
}

bool FocusScope::isFocused() const
{
    return d->m_isFocused;
}

WidgetType *FocusScope::focusedWidget() const
{
    return d->m_lastFocusedInScope;
}

void FocusScope::focus(Qt::FocusReason reason)
{
    if (d->m_lastFocusedInScope) {
        d->m_lastFocusedInScope->setFocus(reason);
    } else {
        if (auto frame = qobject_cast<Frame*>(d->m_thisWidget)) {
            if (DockWidgetBase *dw = frame->currentDockWidget()) {
                if (auto guest = dw->widget()) {
                    if (guest->focusPolicy() != Qt::NoFocus)
                        guest->setFocus(reason);
                }
            }
        } else {
            // Not a use case right now
            d->m_thisWidget->setFocus(reason);
        }
    }
}

void FocusScope::Private::setIsFocused(bool is)
{
    if (is != m_isFocused) {
        m_isFocused = is;

        if (is)
            emitDockWidgetFocusChanged();

        if (!m_inCtor) // Hack so we don't call pure-virtual
            /* Q_EMIT */ q->isFocusedChangedCallback();
    }
}

void FocusScope::Private::onFocusObjectChanged(QObject *obj)
{
    auto widget = qobject_cast<WidgetType*>(obj);
    if (!widget) {
        setIsFocused(false);
        return;
    }

    const bool is = isInFocusScope(widget);
    if (is && m_lastFocusedInScope != widget && !qobject_cast<TitleBar*>(obj)) {
        m_lastFocusedInScope = widget;
        setIsFocused(is);
        /* Q_EMIT */ q->focusedWidgetChangedCallback();
    } else {
        setIsFocused(is);
    }
}

bool FocusScope::Private::isInFocusScope(WidgetType *widget) const
{
    WidgetType *p = widget;
    while (p) {
        if (p == m_thisWidget)
            return true;

        p = KDDockWidgets::Private::parentWidget(p);
    }

    return false;
}

void FocusScope::Private::emitDockWidgetFocusChanged()
{
    auto p = qobject_cast<WidgetType*>(qApp->focusObject());
    if (!p) return;

    // Find the nearest DockWidget and send the focusChangedSignal
    while (p) {
        if (auto frame = qobject_cast<Frame*>(p)) {
            // Special case: The focused widget is inside the frame but not inside the dockwidget.
            // For example, it's a line edit in the QTabBar. We still need to send the signal for
            // the current dw in the tab group
            if (auto dw = frame->currentDockWidget()) {
                DockRegistry::self()->setFocusedDockWidget(dw);
            }

            break;
        }

        if (p == m_thisWidget)
            break;

        if (auto dw = qobject_cast<DockWidgetBase*>(p)) {
            DockRegistry::self()->setFocusedDockWidget(dw);
            break;
        }

        p = KDDockWidgets::Private::parentWidget(p);
    }
}
