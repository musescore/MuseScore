/*
  This file is part of KDDockWidgets.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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
#include "Platform.h"
#include "ViewGuard.h"
#include "core/DockWidget.h"
#include "core/Group.h"
#include "DockRegistry.h"
#include "core/Platform_p.h"
#include "core/Logging_p.h"
#include "core/ViewGuard.h"
#include "View.h"
#include "core/views/DockWidgetViewInterface.h"

using namespace KDDockWidgets;
using namespace KDDockWidgets::Core;

class FocusScope::Private
{
public:
    Private(FocusScope *qq, View *thisView)
        : q(qq)
        , m_thisView(thisView)
    {
        auto plat = Platform::instance();
        m_connection = plat->d->focusedViewChanged.connect(&Private::onFocusedViewChanged, this);

        onFocusedViewChanged(plat->focusedView());

        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        m_inCtor = false;
    }

    /// @brief Returns whether the last focused widget is the tab widget itself
    bool lastFocusedIsTabWidget() const
    {
        return m_lastFocusedInScope && !m_lastFocusedInScope->isNull()
            && m_lastFocusedInScope->is(ViewType::Stack);
    }

    ~Private();

    void setIsFocused(bool);
    void onFocusedViewChanged(std::shared_ptr<View> view);
    bool isInFocusScope(std::shared_ptr<View> view) const;

    FocusScope *const q;
    ViewGuard m_thisView;
    bool m_isFocused = false;
    bool m_inCtor = true;
    std::shared_ptr<View> m_lastFocusedInScope;
    KDBindings::ConnectionHandle m_connection;
};

FocusScope::Private::~Private()
{
    m_connection.disconnect();
}

FocusScope::FocusScope(View *thisView)
    : d(new Private(this, thisView))
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

void FocusScope::focus(Qt::FocusReason reason)
{
    // Note: For QtQuick, qGuiApp->focusObject().isVisible() can be false! Because QtQuick.

    if (d->m_lastFocusedInScope && !d->m_lastFocusedInScope->isNull() && d->m_lastFocusedInScope->isVisible()
        && !d->lastFocusedIsTabWidget()) {
        // When we focus the FocusScope, we give focus to the last focused widget, but let's
        // do better than focusing a tab widget. The tab widget itself being focused isn't
        // very useful.
        d->m_lastFocusedInScope->setFocus(reason);
    } else {
        if (auto group = d->m_thisView->asGroupController()) {
            if (auto dw = group->currentDockWidget()) {
                if (auto dwView = dynamic_cast<Core::DockWidgetViewInterface *>(dw->view())) {
                    if (auto candidate = dwView->focusCandidate()) {
                        if (candidate->focusPolicy() != Qt::NoFocus) {
                            KDDW_DEBUG("FocusScope::focus: Setting focus on candidate!");
                            candidate->setFocus(reason);
                        } else {
                            KDDW_DEBUG("FocusScope::focus: Candidate has no focus policy");
                        }
                    } else {
                        KDDW_DEBUG("FocusScope::focus: Candidate not found");
                    }
                } else {
                    KDDW_DEBUG("FocusScope::focus: Dw doesn't have view");
                }
            } else {
                KDDW_DEBUG("FocusScope::focus: Group doesn't have current DW");
            }
        } else {
            // Not a use case right now
            KDDW_DEBUG("FocusScope::focus: No group found");
            d->m_thisView->setFocus(reason);
        }
    }
}

void FocusScope::Private::setIsFocused(bool is)
{
    if (is != m_isFocused) {
        m_isFocused = is;

        if (!m_inCtor) // Hack so we don't call pure-virtual
            /* Q_EMIT */ q->isFocusedChangedCallback();
    }
}

void FocusScope::Private::onFocusedViewChanged(std::shared_ptr<View> view)
{
    if (!view || view->isNull()) {
        setIsFocused(false);
        return;
    }

    const bool is = isInFocusScope(view);
    const bool focusViewChanged = !m_lastFocusedInScope || m_lastFocusedInScope->isNull()
        || !m_lastFocusedInScope->equals(view);
    if (is && focusViewChanged && !view->is(ViewType::TitleBar)) {
        m_lastFocusedInScope = view;
        setIsFocused(is);
        /* Q_EMIT */ q->focusedWidgetChangedCallback();
    } else {
        setIsFocused(is);
    }
}

bool FocusScope::Private::isInFocusScope(std::shared_ptr<View> view) const
{
    if (m_thisView.isNull())
        return false;

    auto p = (view && !view->isNull()) ? view : std::shared_ptr<View>();
    while (p) {
        if (p->handle() == m_thisView->handle())
            return true;

        p = p->parentView();
    }

    return false;
}
