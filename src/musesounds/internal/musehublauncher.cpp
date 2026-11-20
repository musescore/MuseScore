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
#include "musehublauncher.h"

#include "global/log.h"

using namespace mu::musesounds;
using namespace muse;

void MuseHubLauncher::open(MuseHubPage page, const std::string& utmContent)
{
    tryOpenMuseHub(configuration()->museHubUrls(page, utmContent));
}

void MuseHubLauncher::openSoundLibrary(const String& code, const std::string& utmContent)
{
    tryOpenMuseHub(configuration()->museHubUrls(MuseHubPage::SoundLibrary, utmContent, code));
}

void MuseHubLauncher::tryOpenMuseHub(const MuseHubUrls& urls)
{
    LOGI() << "Opening MuseHub: " << urls.museHubUri;

    const UriQuery museHubUri(urls.museHubUri);
    async::Promise<Ret> canOpenPromise = interactive()->canOpenApp(museHubUri);

    canOpenPromise.onResolve(this, [this, museHubUri, webUrl = urls.webUrl](const Ret& ret) {
        if (!ret) {
            LOGW() << "MuseHub can't be opened, opening web URL instead: " << ret.toString();
            interactive()->openUrl(webUrl);
            return;
        }

        openMuseHubApp(museHubUri, webUrl);
    });

    canOpenPromise.onReject(this, [this, webUrl = urls.webUrl](int, const std::string& err) {
        LOGW() << "MuseHub can't be opened, opening web URL instead: " << err;
        interactive()->openUrl(webUrl);
    });
}

void MuseHubLauncher::openMuseHubApp(const UriQuery& museHubUri, const std::string& webUrl)
{
    async::Promise<Ret> promise = interactive()->openApp(museHubUri);

    promise.onResolve(this, [this, webUrl](const Ret& ret) {
        if (!ret) {
            LOGW() << "Failed to open MuseHub, opening web URL instead: " << ret.toString();
            interactive()->openUrl(webUrl);
        }
    });

    promise.onReject(this, [this, webUrl](int, const std::string& err) {
        LOGW() << "Failed to open MuseHub, opening web URL instead: " << err;
        interactive()->openUrl(webUrl);
    });
}
