/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

#include "modularity/imoduleinterface.h"

#include "types/ret.h"
#include "types/uri.h"
#include "async/promise.h"
#include "io/path.h"

namespace muse {
class IPlatformInteractive : MODULE_GLOBAL_INTERFACE
{
    INTERFACE_ID(IPlatformInteractive)

public:
    virtual ~IPlatformInteractive() = default;

    virtual Ret openUrl(const std::string& url) const = 0;
    virtual Ret openUrl(const QUrl& url) const = 0;

    virtual Ret isAppExists(const std::string& appIdentifier) const = 0;
    virtual Ret canOpenApp(const UriQuery& uri) const = 0;
    virtual async::Promise<Ret> openApp(const UriQuery& uri) const = 0;

    /// Opens a file browser at the parent directory of filePath,
    /// and selects the file at filePath on OSs that support it
    virtual Ret revealInFileBrowser(const io::path_t& filePath) const = 0;
};
}
