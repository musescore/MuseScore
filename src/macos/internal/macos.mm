//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "macos.h"

#include "log.h"

#include <Cocoa/Cocoa.h>

using namespace mu::macos;
using namespace mu::async;

using ObserverToken = id<NSObject>;

std::map<std::string, std::list<ObserverToken>> observerTokens;

void MacOS::init() {
    observeDistributedNotifications("AppleInterfaceThemeChangedNotification", [this]() {
        m_systemDarkModeSwitched.send(isSystemDarkModeOn());
    });
}

// Returns a pointer to the observer token
void* MacOS::observeDistributedNotifications(std::string name, std::function<void()> f) const {
    NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
    NSDistributedNotificationCenter* n = [NSDistributedNotificationCenter defaultCenter];
    ObserverToken token = [n addObserverForName:nsName object:nil queue:nil usingBlock:^(NSNotification*) { f(); }];
    observerTokens[name].push_back(token);
    LOGI() << "Observing notifications for name \"" << name << "\"";
    return (void*)token;
}

void MacOS::stopObservingDistributedNotificationsForName(std::string name) {
    NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
    NSDistributedNotificationCenter* n = [NSDistributedNotificationCenter defaultCenter];
    for (auto token : observerTokens[name]) {
        [n removeObserver:token name:nsName object:nil];
    }
    observerTokens.erase(name);
    LOGI() << "Stopped observing notifications for name \"" << name << "\"";
}

void MacOS::stopObservingDistributedNotificationsForToken(void* vToken) {
    auto token = (ObserverToken)vToken;
    NSDistributedNotificationCenter* n = [NSDistributedNotificationCenter defaultCenter];
    [n removeObserver:token];
    for (auto& [name, tokens] : observerTokens) {
        tokens.remove(token);
    }
}

void MacOS::stopObservingDistributedAllNotifications() {
    NSDistributedNotificationCenter* n = [NSDistributedNotificationCenter defaultCenter];
    for (auto const& [name, tokens] : observerTokens) {
        NSString* nsName = [NSString stringWithUTF8String:name.c_str()];
        for (auto token : tokens) {
            [n removeObserver:token name:nsName object:nil];
        }
        LOGI() << "Stopped observing notifications for name \"" << name << "\"";
    }
    observerTokens.clear();
}

bool MacOS::isSystemDarkModeOn() const {
    NSString* systemMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
    return ([systemMode isEqualToString:@"Dark"]);
}

Channel<bool> MacOS::systemDarkModeSwitched() const
{
    return m_systemDarkModeSwitched;
}

void MacOS::setAppAppearanceDark(bool flag) {
    [NSApp setAppearance:[NSAppearance appearanceNamed:flag ? NSAppearanceNameDarkAqua : NSAppearanceNameAqua]];
}
