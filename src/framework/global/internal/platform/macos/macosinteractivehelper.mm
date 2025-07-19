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

Ret MacOSInteractiveHelper::isAppExists(const std::string& appIdentifier)
{
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    NSURL* appURL = [workspace URLForApplicationWithBundleIdentifier:@(appIdentifier.c_str())];
    return appURL != nil;
}

Ret MacOSInteractiveHelper::canOpenApp(const Uri& uri)
{
    if (__builtin_available(macOS 10.15, *)) {
        NSString* nsUrlString = [NSString stringWithUTF8String:uri.toString().c_str()];
        if (nsUrlString == nil) {
            return make_ret(Ret::Code::InternalError, std::string("Invalid UTF-8 string passed as URI"));
        }

        NSURL* nsUrl = [NSURL URLWithString:nsUrlString];
        if (nsUrl == nil) {
            return make_ret(Ret::Code::InternalError, std::string("Invalid URI"));
        }

        NSURL* appURL = [[NSWorkspace sharedWorkspace] URLForApplicationToOpenURL:nsUrl];
        return appURL != nil;
    } else {
        return false;
    }
}

async::Promise<Ret> MacOSInteractiveHelper::openApp(const Uri& uri)
{
    return Promise<Ret>([&uri](auto resolve, auto reject) {
        if (__builtin_available(macOS 10.15, *)) {
            NSString* nsUrlString = [NSString stringWithUTF8String:uri.toString().c_str()];
            if (nsUrlString == nil) {
                return reject(int(Ret::Code::InternalError), "Invalid UTF-8 string passed as URI");
            }

            NSURL* nsUrl = [NSURL URLWithString:nsUrlString];
            if (nsUrl == nil) {
                return reject(int(Ret::Code::InternalError), "Invalid URI");
            }

            auto configuration = [NSWorkspaceOpenConfiguration configuration];
            [configuration setPromptsUserIfNeeded:NO];
            [[NSWorkspace sharedWorkspace]
             openURL: nsUrl
             configuration: configuration
             completionHandler: ^(NSRunningApplication*, NSError* error) {
                 if (error) {
                     std::string errorStr = [[error description] UTF8String];
                     (void)reject(int(Ret::Code::InternalError), errorStr);
                 } else {
                     (void)resolve(make_ok());
                 }
             }
            ];

            return Promise<Ret>::Result::unchecked();
        } else {
            return reject(int(Ret::Code::NotSupported), "macOS 10.15 or later is required");
        }
    });
}
