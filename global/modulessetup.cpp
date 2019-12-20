#include "modulessetup.h"
#include "config.h"

#ifdef BUILD_TELEMETRY_MODULE
#include "telemetrysetup.h"
#endif

ModulesSetup::ModulesSetup()
      {
#ifdef BUILD_TELEMETRY_MODULE
      m_modulesSetupList << new TelemetrySetup();
#endif
      }

void ModulesSetup::setup()
      {
      for (AbstractModuleSetup* moduleSetup : m_modulesSetupList) {
            moduleSetup->setup();
            }
      }
