#ifndef MODULESSETUP_H
#define MODULESSETUP_H

#include <QList>
#include "interfaces/abstractmodulesetup.h"

class ModulesSetup
{
public:
      static ModulesSetup* instance() {
            static ModulesSetup s;
            return &s;
            }

      void setup();

private:
      Q_DISABLE_COPY(ModulesSetup)

      ModulesSetup();

      QList<AbstractModuleSetup*> m_modulesSetupList;
};

#endif // MODULESSETUP_H
