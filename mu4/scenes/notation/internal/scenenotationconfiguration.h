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
#ifndef MU_NOTATIONSCENE_SCENENOTATIONCONFIGURATION_H
#define MU_NOTATIONSCENE_SCENENOTATIONCONFIGURATION_H

#include "../iscenenotationconfiguration.h"

namespace mu {
namespace scene {
namespace notation {
class SceneNotationConfiguration : public ISceneNotationConfiguration
{
public:

    void init();

    QColor backgroundColor() const override;
    async::Channel<QColor> backgroundColorChanged() const override;

    QColor foregroundColor() const override;
    QColor defaultForegroundColor() const override;
    async::Channel<QColor> foregroundColorChanged() const override;

    QColor playbackCursorColor() const override;

    int selectionProximity() const override;

private:
    async::Channel<QColor> m_backgroundColorChanged;
    async::Channel<QColor> m_foregroundColorChanged;
};
}
}
}

#endif // MU_NOTATIONSCENE_SCENENOTATIONCONFIGURATION_H
