/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#include "macosinteractivehelper.h"

#include <QUrl>
#include <QStandardPaths>

#include <Cocoa/Cocoa.h>

#include "log.h"

using namespace muse;
using namespace muse::async;

bool MacOSInteractiveHelper::revealInFinder(const io::path_t& filePath)
{
    NSURL* fileUrl = QUrl::fromLocalFile(filePath.toQString()).toNSURL();

    [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:@[fileUrl]];

    return true;
}

async::Promise<Ret> MacOSInteractiveHelper::openApp(const std::string& appIdentifier)
{
    return Promise<Ret>([&appIdentifier](auto resolve, auto reject) {
        NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
        NSString* nsAppIdentifier = [NSString stringWithUTF8String:appIdentifier.c_str()];
        NSURL* appURL = [NSURL URLWithString: nsAppIdentifier];
        if (!appURL.scheme || [appURL.scheme isEqualToString:@""]) {
            appURL = [workspace URLForApplicationWithBundleIdentifier:nsAppIdentifier];
        }

        auto configuration = [NSWorkspaceOpenConfiguration configuration];
        [configuration setPromptsUserIfNeeded:NO];
        [workspace openURL: appURL
         configuration: configuration
         completionHandler: ^(NSRunningApplication*, NSError* error) {
             if (error) {
                 std::string errorStr = [[error description] UTF8String];
                 Ret ret = make_ret(Ret::Code::InternalError, errorStr);
                 (void)reject(ret.code(), ret.text());
             } else {
                 (void)resolve(make_ok());
             }
         }
        ];

        return Promise<Ret>::Result::unchecked();
    });
}
