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
#include "macosvstmodulepaths.h"

#include <set>
#include <string>

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>

#include "log.h"

using namespace muse;
using namespace muse::vst;

using VisitedDirs = std::set<std::string>;

static void findModulesInDirectory(NSURL* dirUrl, io::paths_t& result, VisitedDirs& visitedDirs)
{
    dirUrl = [dirUrl URLByResolvingSymlinksInPath];
    if (!dirUrl) {
        return;
    }

    const char* dirPath = [dirUrl.path UTF8String];
    if (!dirPath) {
        return;
    }

    if (!visitedDirs.insert(dirPath).second) {
        LOGW() << "Directory already scanned, skipping: " << dirPath;
        return;
    }

    NSDirectoryEnumerator* enumerator = [[NSFileManager defaultManager] enumeratorAtURL:dirUrl
                                         includingPropertiesForKeys:nil
                                         options:NSDirectoryEnumerationSkipsPackageDescendants
                                         errorHandler:nil];

    for (NSURL* url in enumerator) {
        @autoreleasepool {
            if ([[[url lastPathComponent] pathExtension] isEqualToString:@"vst3"]) {
                CFArrayRef archs = CFBundleCopyExecutableArchitecturesForURL((__bridge CFURLRef)url);
                if (archs) {
                    result.emplace_back([url.path UTF8String]);
                    CFRelease(archs);
                }
                continue;
            }

            id resValue = nil;
            if (![url getResourceValue:&resValue forKey:NSURLIsSymbolicLinkKey error:nil]) {
                continue;
            }
            if (![static_cast<NSNumber*>(resValue) boolValue]) {
                continue;
            }

            NSURL* resolvedUrl = [url URLByResolvingSymlinksInPath];
            if (![resolvedUrl getResourceValue:&resValue forKey:NSURLIsDirectoryKey error:nil]) {
                continue;
            }
            if (![static_cast<NSNumber*>(resValue) boolValue]) {
                continue;
            }

            findModulesInDirectory(resolvedUrl, result, visitedDirs);
        }
    }
}

static void getModules(NSSearchPathDomainMask domain, io::paths_t& result, VisitedDirs& visitedDirs)
{
    NSURL* libraryUrl = [[NSFileManager defaultManager] URLForDirectory:NSLibraryDirectory
                         inDomain:domain
                         appropriateForURL:nil
                         create:NO
                         error:nil];
    if (!libraryUrl) {
        return;
    }

    NSURL* vst3Url = [libraryUrl URLByAppendingPathComponent:@"Audio/Plug-Ins/VST3"];
    if (!vst3Url) {
        return;
    }

    findModulesInDirectory(vst3Url, result, visitedDirs);
}

static void getApplicationModules(io::paths_t& result, VisitedDirs& visitedDirs)
{
    NSURL* bundleUrl = [[NSBundle mainBundle] bundleURL];
    if (!bundleUrl) {
        return;
    }

    NSURL* vst3Url = [bundleUrl URLByAppendingPathComponent:@"Contents/VST3"];
    if (!vst3Url) {
        return;
    }

    findModulesInDirectory(vst3Url, result, visitedDirs);
}

io::paths_t MacOSVstModulePaths::getModulePaths()
{
    io::paths_t result;
    VisitedDirs visitedDirs;

    @autoreleasepool {
        getModules(NSUserDomainMask, result, visitedDirs);
        getModules(NSLocalDomainMask, result, visitedDirs);
        getApplicationModules(result, visitedDirs);
    }

    return result;
}
