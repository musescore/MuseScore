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
#ifndef MU_AUDIO_AUDIOMODULE_H
#define MU_AUDIO_AUDIOMODULE_H

#include "modularity/imodulesetup.h"
#include "async/asyncable.h"

namespace mu::audio {
class AudioModule : public framework::IModuleSetup, public async::Asyncable
{
public:
    std::string moduleName() const override;

    void registerExports() override;
    void registerUiTypes() override;
    void onInit(const framework::IApplication::RunMode& mode) override;
    void onDeinit() override;
};
}

#endif // MU_AUDIO_AUDIOMODULE_H
