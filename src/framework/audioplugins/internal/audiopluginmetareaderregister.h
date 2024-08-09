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

#pragma once

#include "../iaudiopluginmetareaderregister.h"

namespace muse::audioplugins {
class AudioPluginMetaReaderRegister : public IAudioPluginMetaReaderRegister
{
public:
    const std::vector<IAudioPluginMetaReaderPtr>& readers() const override;
    void registerReader(IAudioPluginMetaReaderPtr reader) override;

private:
    std::vector<IAudioPluginMetaReaderPtr> m_readers;
};
}
