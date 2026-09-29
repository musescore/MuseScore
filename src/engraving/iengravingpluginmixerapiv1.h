/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include <QVariantList>

#include "async/promise.h"
#include "engraving/types/types.h"
#include "modularity/imoduleinterface.h"

namespace mu::engraving
{
    class IEngravingPluginMixerApi : MODULE_CONTEXT_INTERFACE
    {
        INTERFACE_ID(IEngravingPluginMixerApi)

    public:
        virtual ~IEngravingPluginMixerApi() = default;

        virtual float volume(const InstrumentTrackId &trackId) const = 0;
        virtual void setVolume(const InstrumentTrackId &trackId, float volume) = 0;
        virtual float balance(const InstrumentTrackId &trackId) const = 0;
        virtual void setBalance(const InstrumentTrackId &trackId, float balance) = 0;
        virtual bool muted(const InstrumentTrackId &trackId) const = 0;
        virtual void setMuted(const InstrumentTrackId &trackId, bool muted) = 0;
        virtual bool solo(const InstrumentTrackId &trackId) const = 0;
        virtual void setSolo(const InstrumentTrackId &trackId, bool solo) = 0;

        virtual muse::async::Promise<QVariantList> availableSounds(const InstrumentTrackId &trackId) const = 0;
        virtual muse::async::Promise<bool> setSound(const InstrumentTrackId &trackId, const QString &soundId) = 0;
    };
}