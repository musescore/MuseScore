/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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
#pragma once

#include "global/async/asyncable.h"

#include "global/modularity/ioc.h"
#include "global/iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "context/iglobalcontext.h"
#include "audio/main/istartaudiocontroller.h"
#include "audio/main/isoundfontcontroller.h"

namespace mu::webbridge {
class WebApi : public muse::async::Asyncable
{
    inline static muse::GlobalInject<muse::IInteractive> interactive;
    inline static muse::GlobalInject<muse::actions::IActionsDispatcher> dispatcher;
    inline static muse::GlobalInject<mu::context::IGlobalContext> globalContext;
    inline static muse::GlobalInject<muse::audio::IStartAudioController> startAudioController;
    inline static muse::GlobalInject<muse::audio::ISoundFontController> soundFontController;

public:

    static WebApi* instance();

    void init();
    void deinit();

    void load(const void* source, unsigned int len);
    void addSoundFont(const std::string& uri);
    void startAudioProcessing();

private:

    WebApi() = default;

    void onProjectSaved(const muse::io::path_t& path, mu::project::SaveMode mode);

    project::INotationProjectPtr m_currentProject;
};
}
