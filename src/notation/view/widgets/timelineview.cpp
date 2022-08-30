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

#include "timelineview.h"

#include "timeline.h"

#include <QApplication>
#include <QSplitter>

#include "log.h"

namespace mu::notation {
class TimelineAdapter : public QSplitter, public uicomponents::IDisplayableWidget
{
public:
    TimelineAdapter()
        : QSplitter(nullptr)
    {
        setFocusPolicy(Qt::NoFocus);
        setHandleWidth(0);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setObjectName("TimelineAdapter");

        m_msTimeline = new Timeline(this);
    }

    void updateView()
    {
        m_msTimeline->updateGridFromCmdState();
    }

    void setNotation(INotationPtr notation)
    {
        m_msTimeline->setNotation(notation);
    }

private:
    QWidget* qWidget() override
    {
        return this;
    }

    bool handleEvent(QEvent* e) override
    {
        if (QMouseEvent* me = dynamic_cast<QMouseEvent*>(e)) {
            return handleMouseEvent(me);
        }

        return m_msTimeline->handleEvent(e);
    }

    bool handleMouseEvent(QMouseEvent* event)
    {
        QPoint pos = event ? event->pos() : QPoint();

        if (event->type() == QEvent::MouseButtonPress) {
            m_mouseDownWidget = childAt(pos);
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_mouseDownWidget = nullptr;
        }

        if (QWidget* receiver = m_mouseDownWidget ? m_mouseDownWidget : childAt(pos)) {
            event->setLocalPos(receiver->mapFrom(this, pos));
            return qApp->notify(receiver, event);
        }

        return false;
    }

    Timeline* m_msTimeline = nullptr;
    QWidget* m_mouseDownWidget = nullptr;
};
}

using namespace mu::notation;

TimelineView::TimelineView(QQuickItem* parent)
    : WidgetView(parent)
{
}

void TimelineView::componentComplete()
{
    WidgetView::componentComplete();

    auto timeline = std::make_shared<TimelineAdapter>();

    auto updateView = [this, timeline]() {
        update();

        if (timeline) {
            timeline->updateView();
        }
    };

    auto initTimeline = [this, updateView, timeline] {
        INotationPtr notation = globalContext()->currentNotation();
        timeline->setNotation(notation);

        updateView();

        if (!notation) {
            return;
        }

        notation->undoStack()->stackChanged().onNotify(this, [=] {
            updateView();
        });

        notation->interaction()->selectionChanged().onNotify(this, [=] {
            updateView();
        });
    };

    globalContext()->currentNotationChanged().onNotify(this, [initTimeline]() {
        initTimeline();
    });

    setWidget(timeline);

    initTimeline();
}
