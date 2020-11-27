#ifndef OLDSCOREHANDLER_H
#define OLDSCOREHANDLER_H

#include "migration/iscoremigrationhandler.h"

class StyleDefaultsHandler : public IScoreMigrationHandler
      {
    public:
      StyleDefaultsHandler() = default;

      bool handle(Ms::Score* score) override;
      };

#endif // OLDSCOREHANDLER_H
