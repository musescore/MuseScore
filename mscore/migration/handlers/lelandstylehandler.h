#ifndef LELANDSTYLEHANDLER_H
#define LELANDSTYLEHANDLER_H

#include "migration/iscoremigrationhandler.h"

class LelandStyleHandler : public IScoreMigrationHandler
      {
    public:
      LelandStyleHandler() = default;

      bool handle(Ms::Score* score) override;
      };

#endif // LELANDSTYLEHANDLER_H
