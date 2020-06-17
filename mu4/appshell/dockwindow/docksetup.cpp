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

#include "docksetup.h"

#include <QtQml>

#include "dockwindow.h"
#include "dockpage.h"
#include "docktoolbar.h"
#include "dockpanel.h"
#include "dockcentral.h"
#include "dockstatusbar.h"

using namespace mu::dock;

DockSetup::DockSetup()
{
}

void DockSetup::registerQmlTypes()
{
    qmlRegisterType<DockWindow>("MuseScore.Dock", 1, 0, "DockWindow");
    qmlRegisterType<DockPage>("MuseScore.Dock", 1, 0, "DockPage");
    qmlRegisterType<DockToolBar>("MuseScore.Dock", 1, 0, "DockToolBar");
    qmlRegisterType<DockPanel>("MuseScore.Dock", 1, 0, "DockPanel");
    qmlRegisterType<DockCentral>("MuseScore.Dock", 1, 0, "DockCentral");
    qmlRegisterType<DockStatusBar>("MuseScore.Dock", 1, 0, "DockStatusBar");
}
