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

#include "lyricsexportconfiguration.h"

#include "settings.h"

using namespace muse;
using namespace mu::iex::lrcexport;

static const std::string module_name("iex_lyricsexport");

static const Settings::Key LRC_USE_ENHANCED_FORMAT_KEY(module_name, "export/lrc/useEnhancedFormat");

void LyricsExportConfiguration::init()
{
    settings()->setDefaultValue(LRC_USE_ENHANCED_FORMAT_KEY, Val(true));
    settings()->valueChanged(LRC_USE_ENHANCED_FORMAT_KEY).onReceive(this, [this](const Val& val) {
        m_lrcUseEnhancedFormatChanged.send(val.toBool());
    });
}

bool LyricsExportConfiguration::lrcUseEnhancedFormat() const
{
    return settings()->value(LRC_USE_ENHANCED_FORMAT_KEY).toBool();
}

void LyricsExportConfiguration::setLrcUseEnhancedFormat(bool value)
{
    settings()->setSharedValue(LRC_USE_ENHANCED_FORMAT_KEY, Val(value));
}

async::Channel<bool> LyricsExportConfiguration::lrcUseEnhancedFormatChanged() const
{
    return m_lrcUseEnhancedFormatChanged;
}
