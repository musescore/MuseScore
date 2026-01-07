/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#include "async/channel.h"

#include "modularity/imoduleinterface.h"

namespace mu::iex::lrcexport {
class ILyricsExportConfiguration : MODULE_GLOBAL_EXPORT_INTERFACE
{
    INTERFACE_ID(ILyricsExportConfiguration)

public:
    virtual ~ILyricsExportConfiguration() = default;

    virtual bool lrcUseEnhancedFormat() const = 0;
    virtual void setLrcUseEnhancedFormat(bool value) = 0;
    virtual muse::async::Channel<bool> lrcUseEnhancedFormatChanged() const = 0;
};
}
