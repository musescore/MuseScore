//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2019 MuseScore BVBA and others
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

#include "modulessetup.h"
#include "config.h"

#include "framework/ui/uimodule.h"
#include "framework/uicomponents/uicomponentsmodule.h"

#ifdef BUILD_TELEMETRY_MODULE
#include "framework/telemetry/telemetrysetup.h"
#endif

#ifdef AVSOMR
#include "avsomr/avsomrsetup.h"
#endif

#include "inspectors/inspectorssetup.h"

//---------------------------------------------------------
//   ModulesSetup
//---------------------------------------------------------

ModulesSetup::ModulesSetup()
{
    m_modulesSetupList
        << new mu::framework::UiModule()
        << new mu::framework::UiComponentsModule()
#ifdef BUILD_TELEMETRY_MODULE
        << new TelemetrySetup()
#endif
#ifdef AVSOMR
        << new Ms::Avs::AvsOmrSetup()
#endif
        << new InspectorsSetup();
}

//---------------------------------------------------------
//   setup
//---------------------------------------------------------

void ModulesSetup::setup()
{
    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->registerExports();
    }

    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->resolveImports();

        m->registerResources();
        m->registerUiTypes();
    }

    //! NOTE Need to move to the place where the application finishes initializing
    for (mu::framework::IModuleSetup* m : m_modulesSetupList) {
        m->onStartInit();
    }
}
