/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#include "audioworkerconfiguration.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::audio::worker;

audioch_t AudioWorkerConfiguration::audioChannelsCount() const
{
    return configuration()->audioChannelsCount();
}

samples_t AudioWorkerConfiguration::samplesToPreallocate() const
{
    return configuration()->samplesToPreallocate();
}

async::Channel<samples_t> AudioWorkerConfiguration::samplesToPreallocateChanged() const
{
    return configuration()->samplesToPreallocateChanged();
}

bool AudioWorkerConfiguration::autoProcessOnlineSoundsInBackground() const
{
    return configuration()->autoProcessOnlineSoundsInBackground();
}

async::Channel<bool> AudioWorkerConfiguration::autoProcessOnlineSoundsInBackgroundChanged() const
{
    return configuration()->autoProcessOnlineSoundsInBackgroundChanged();
}
