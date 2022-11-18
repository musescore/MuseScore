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
#ifndef MU_AUDIO_AUDIOMODULE_H
#define MU_AUDIO_AUDIOMODULE_H

#include "modularity/imodulesetup.h"
#include "async/asyncable.h"

#include "iaudiodriver.h"

namespace mu::audio {
class AudioModule : public modularity::IModuleSetup, public async::Asyncable
{
public:
    AudioModule();

    std::string moduleName() const override;

    void registerExports() override;
    void registerResources() override;
    void registerUiTypes() override;
    void onInit(const framework::IApplication::RunMode& mode) override;
    void onDeinit() override;
private:
    void setupAudioDriver(const framework::IApplication::RunMode& mode);
    void setupAudioWorker(const IAudioDriver::Spec& activeSpec);
};
}

#endif // MU_AUDIO_AUDIOMODULE_H
