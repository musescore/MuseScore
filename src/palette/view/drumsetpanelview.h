/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "uicomponents/view/widgetview.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "actions/iactionsdispatcher.h"
#include "notation/inotationconfiguration.h"
#include "engraving/iengravingconfiguration.h"

namespace mu::palette {
class DrumsetPaletteAdapter;
class DrumsetPanelView : public muse::uicomponents::WidgetView, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(context::IGlobalContext, globalContext)
    INJECT(muse::actions::IActionsDispatcher, dispatcher)
    INJECT(notation::INotationConfiguration, notationConfiguration)
    INJECT(engraving::IEngravingConfiguration, engravingConfiguration)

    Q_PROPERTY(QString pitchName READ pitchName NOTIFY pitchNameChanged)

public:
    explicit DrumsetPanelView(QQuickItem* parent = nullptr);

    QString pitchName() const;

    Q_INVOKABLE void customizeKit();

signals:
    void pitchNameChanged();

private:
    void componentComplete() override;

    void initDrumsetPalette();
    void updateColors();

    void setPitchName(const QString& name);

    QString m_pitchName;

    std::shared_ptr<DrumsetPaletteAdapter> m_adapter;
};
}

#endif // MU_PALETTE_DRUMSETOOLSPANELVIEW_H
