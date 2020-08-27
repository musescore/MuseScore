//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_PALETTE_PALETTEROOTMODEL_H
#define MU_PALETTE_PALETTEROOTMODEL_H

#include <QObject>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "../ipaletteadapter.h"
#include "../internal/palette/paletteworkspace.h"

namespace mu {
namespace palette {
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
}

#endif // MU_PALETTE_PALETTEROOTMODEL_H
