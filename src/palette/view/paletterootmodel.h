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
#ifndef MU_PALETTE_PALETTEROOTMODEL_H
#define MU_PALETTE_PALETTEROOTMODEL_H

#include <QObject>

#include "actions/actionable.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "internal/palette/paletteprovider.h"
#include "actions/iactionsdispatcher.h"

namespace mu::palette {
class PaletteRootModel : public QObject, public actions::Actionable, public async::Asyncable
{
    Q_OBJECT

    INJECT(palette, IPaletteProvider, paletteProvider)
    INJECT(palette, actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(Ms::PaletteProvider* paletteProvider READ paletteProvider_property CONSTANT)

    Q_PROPERTY(bool paletteEnabled READ paletteEnabled NOTIFY paletteEnabledChanged)
    Q_PROPERTY(bool needShowShadowOverlay READ needShowShadowOverlay NOTIFY needShowShadowOverlayChanged)

public:
    explicit PaletteRootModel(QObject* parent = nullptr);

    Ms::PaletteProvider* paletteProvider_property() const;

    bool paletteEnabled() const;
    bool needShowShadowOverlay() const;

signals:
    void paletteSearchRequested();
    void paletteEnabledChanged();
    void needShowShadowOverlayChanged();

private:
    bool m_needShowShadowOverlay = false;
};
}

#endif // MU_PALETTE_PALETTEROOTMODEL_H
