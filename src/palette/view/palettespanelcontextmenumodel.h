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
#ifndef MU_PALETTE_PALETTESPANELCONTEXTMENUMODEL_H
#define MU_PALETTE_PALETTESPANELCONTEXTMENUMODEL_H

#include "uicomponents/view/abstractmenumodel.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "ipaletteconfiguration.h"
#include "actions/iactionsdispatcher.h"

namespace mu::palette {
class PalettesPanelContextMenuModel : public muse::uicomponents::AbstractMenuModel, public muse::actions::Actionable
{
    Q_OBJECT

    INJECT(IPaletteConfiguration, configuration)
    INJECT(muse::actions::IActionsDispatcher, dispatcher)

public:
    explicit PalettesPanelContextMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

signals:
    void expandCollapseAllRequested(bool expand);

private:
    muse::uicomponents::MenuItem* createIsSingleClickToOpenPaletteItem();
    muse::uicomponents::MenuItem* createIsSinglePaletteItem();
    muse::uicomponents::MenuItem* createIsDragEnabledItem();
    muse::uicomponents::MenuItem* createExpandCollapseAllItem(bool expand);
};
}

#endif // MU_PALETTE_PALETTESPANELCONTEXTMENUMODEL_H
