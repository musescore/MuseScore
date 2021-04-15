/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
#ifndef MU_PALETTE_PALETTEROOTMODEL_H
#define MU_PALETTE_PALETTEROOTMODEL_H

#include <QObject>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "../ipaletteadapter.h"
#include "../internal/palette/paletteworkspace.h"

namespace mu::palette {
class PaletteRootModel : public QObject, async::Asyncable
{
    Q_OBJECT

    INJECT(palette, IPaletteAdapter, adapter)

    Q_PROPERTY(bool paletteEnabled READ paletteEnabled NOTIFY paletteEnabledChanged)
    Q_PROPERTY(Ms::PaletteWorkspace* paletteWorkspace READ paletteWorkspace CONSTANT)

    Q_PROPERTY(bool shadowOverlay READ shadowOverlay NOTIFY shadowOverlayChanged)

public:
    PaletteRootModel(QObject* parent = nullptr);

    bool paletteEnabled() const;
    Ms::PaletteWorkspace* paletteWorkspace() const;
    bool shadowOverlay() const;

signals:
    void paletteEnabledChanged(bool paletteEnabled);
    void paletteSearchRequested();
    void shadowOverlayChanged(bool shadowOverlay);
    void elementDraggedToScoreView();

private:
    bool m_paletteEnabled = true;
    bool m_shadowOverlay = false;
};
}

#endif // MU_PALETTE_PALETTEROOTMODEL_H
