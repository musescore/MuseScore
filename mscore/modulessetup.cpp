#include "modulessetup.h"

#include "telemetrysetup.h"

ModulesSetup::ModulesSetup()
{
    m_modulesSetupList << new TelemetrySetup();
}

void ModulesSetup::setup()
{
    for (AbstractModuleSetup* moduleSetup : m_modulesSetupList) {
         moduleSetup->setup();
    }
}
