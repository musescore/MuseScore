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
#ifndef MU_NOTATIONSCENE_ISCENENOTATIONCONFIGURATION_H
#define MU_NOTATIONSCENE_ISCENENOTATIONCONFIGURATION_H

#include <QColor>

#include "modularity/imoduleexport.h"
#include "async/channel.h"

namespace mu {
namespace scene {
namespace notation {
class ISceneNotationConfiguration : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(ISceneNotationConfigure)
public:
    virtual ~ISceneNotationConfiguration() = default;

    virtual QColor backgroundColor() const = 0;
    virtual async::Channel<QColor> backgroundColorChanged() const = 0;

    virtual QColor defaultForegroundColor() const = 0;
    virtual QColor foregroundColor() const = 0;
    virtual async::Channel<QColor> foregroundColorChanged() const = 0;

    virtual QColor playbackCursorColor() const = 0;

    virtual int selectionProximity() const = 0;
};
}
}
}

#endif // MU_NOTATIONSCENE_ISCENENOTATIONCONFIGURATION_H
