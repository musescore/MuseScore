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

#include <QSplitter>

namespace mu::notation {
class TimeLineAdapter : public QSplitter, public ui::IDisplayableWidget
{
public:
    TimeLineAdapter()
        : QSplitter(nullptr)
    {
        setFocusPolicy(Qt::NoFocus);
        setHandleWidth(0);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        setObjectName("TimeLineAdapter");

        m_msTimeline = new Ms::Timeline(this, this);
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
        return m_msTimeline->handleEvent(e);
    }

    Ms::Timeline* m_msTimeline = nullptr;
};
}

using namespace mu::notation;

TimeLineView::TimeLineView(QQuickItem* parent)
    : WidgetView(parent)
{
}

void TimeLineView::componentComplete()
{
    WidgetView::componentComplete();

    TimeLineAdapter* timeline = new TimeLineAdapter();

    globalContext()->currentNotationChanged().onNotify(this, [this, timeline]() {
        timeline->setNotation(globalContext()->currentNotation());
    });

    connect(this, &QQuickItem::widthChanged, [timeline, this]() {
        timeline->setMinimumSize(width(), height());
    });

    connect(this, &QQuickItem::heightChanged, [timeline, this]() {
        timeline->setMinimumSize(width(), height());
    });

    setWidget(timeline);
}
