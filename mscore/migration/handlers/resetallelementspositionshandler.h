#ifndef RESETALLELEMENTSPOSITIONSHANDLER_H
#define RESETALLELEMENTSPOSITIONSHANDLER_H

#include "migration/iscoremigrationhandler.h"

class ResetAllElementsPositionsHandler : public IScoreMigrationHandler
      {
   public:
      ResetAllElementsPositionsHandler() = default;

      bool handle(Ms::Score* score) override;
      };

#endif // RESETALLELEMENTSPOSITIONSHANDLER_H
