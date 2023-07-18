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

#ifndef MU_NOTATION_TIMELINEVIEW_H
#define MU_NOTATION_TIMELINEVIEW_H

#include "uicomponents/view/widgetview.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "async/asyncable.h"

namespace mu::notation {
class TimelineView : public uicomponents::WidgetView, public async::Asyncable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)

public:
    explicit TimelineView(QQuickItem* parent = nullptr);

private:
    void componentComplete() override;
};
}

#endif // MU_NOTATION_TIMELINEVIEW_H
