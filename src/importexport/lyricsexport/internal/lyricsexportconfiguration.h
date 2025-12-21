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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "../ilyricsexportconfiguration.h"

#include "async/asyncable.h"

namespace mu::iex::lrcexport {
class LyricsExportConfiguration : public ILyricsExportConfiguration, public muse::async::Asyncable
{
public:
    void init();

    bool lrcUseEnhancedFormat() const override;
    void setLrcUseEnhancedFormat(bool value) override;
    muse::async::Channel<bool> lrcUseEnhancedFormatChanged() const override;

private:
    muse::async::Channel<bool> m_lrcUseEnhancedFormatChanged;
};
}
