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
#ifndef MU_PALETTE_DRUMSETOOLSPANELVIEW_H
#define MU_PALETTE_DRUMSETOOLSPANELVIEW_H

#include "ui/view/widgetview.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"

namespace mu::palette {
class DrumsetPanelView : public ui::WidgetView, public async::Asyncable
{
    Q_OBJECT

    INJECT(palette, context::IGlobalContext, globalContext)
    INJECT(palette, actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(QString pitchName READ pitchName NOTIFY pitchNameChanged)

public:
    explicit DrumsetPanelView(QQuickItem* parent = nullptr);

    QString pitchName() const;

    Q_INVOKABLE void editDrumset();

signals:
    void pitchNameChanged();

private:
    void componentComplete() override;

    QString m_pitchName;
};
}

#endif // MU_PALETTE_DRUMSETOOLSPANELVIEW_H
