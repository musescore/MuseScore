//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2018 Werner Schweer and others
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

#include "cocoabridge.h"
#import <Cocoa/Cocoa.h>

void  CocoaBridge::setAllowsAutomaticWindowTabbing(bool flag)
{
    if ([NSWindow respondsToSelector:@selector(allowsAutomaticWindowTabbing)])
        [NSWindow setAllowsAutomaticWindowTabbing: flag];
}

bool  CocoaBridge::isSystemDarkTheme()
{
    NSString *osxMode = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleInterfaceStyle"];
    return ([osxMode isEqualToString:@"Dark"]);
}
