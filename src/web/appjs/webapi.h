/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
#include "interactive/iinteractive.h"
#include "actions/iactionsdispatcher.h"
#include "project/iprojectsession.h"
#include "project/inotationproject.h"
#include "audio/main/istartaudiocontroller.h"
#include "audio/main/isoundfontcontroller.h"

namespace mu::appjs {
class WebApi : public muse::Contextable, public muse::async::Asyncable
{
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<mu::project::IProjectSession> projectSession = { this };
    inline static muse::GlobalInject<muse::audio::IStartAudioController> startAudioController;
    inline static muse::GlobalInject<muse::audio::ISoundFontController> soundFontController;

public:

    static WebApi* instance();

    void init(const muse::modularity::ContextPtr& ctx);
    void deinit();

    void load(const void* source, unsigned int len);
    void addSoundFont(const std::string& uri);
    void startAudioProcessing();

private:

    WebApi()
        : muse::Contextable(muse::modularity::ContextPtr()) {}

    void onProjectSaved(const muse::io::path_t& path, mu::project::SaveMode mode);

    project::INotationProjectPtr m_currentProject;
};
}
