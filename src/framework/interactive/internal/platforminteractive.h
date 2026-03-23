/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and it under the terms of the GNU General Public License version 3 as
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

#include "../iplatforminteractive.h"

namespace muse {
class PlatformInteractive : public IPlatformInteractive
{
public:

    Ret openUrl(const std::string& url) const override;
    Ret openUrl(const QUrl& url) const override;

    Ret isAppExists(const std::string& appIdentifier) const override;
    Ret canOpenApp(const UriQuery& uri) const override;
    async::Promise<Ret> openApp(const UriQuery& uri) const override;

    Ret revealInFileBrowser(const io::path_t& filePath) const override;
};
}
