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
#ifndef MU_PALETTE_PALETTEROOTMODEL_H
#define MU_PALETTE_PALETTEROOTMODEL_H

#include <QObject>

#include "actions/actionable.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "internal/paletteprovider.h"
#include "actions/iactionsdispatcher.h"

namespace mu::palette {
class PaletteRootModel : public QObject, public muse::actions::Actionable, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(IPaletteProvider, paletteProvider)
    INJECT(muse::actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(mu::palette::PaletteProvider * paletteProvider READ paletteProvider_property CONSTANT)

public:
    explicit PaletteRootModel(QObject* parent = nullptr);
    ~PaletteRootModel() override;

    PaletteProvider* paletteProvider_property() const;

signals:
    void paletteSearchRequested();
    void applyCurrentPaletteElementRequested();
};
}

#endif // MU_PALETTE_PALETTEROOTMODEL_H
