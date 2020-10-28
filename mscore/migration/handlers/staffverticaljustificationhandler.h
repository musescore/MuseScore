#ifndef STAFFVERTICALJUSTIFICATIONHANDLER_H
#define STAFFVERTICALJUSTIFICATIONHANDLER_H

#include "migration/iscoremigrationhandler.h"

class StaffVerticalJustificationHandler : public IScoreMigrationHandler
      {
   public:
      StaffVerticalJustificationHandler() = default;

      bool handle(Ms::Score* score) override;
      };

#endif // STAFFVERTICALJUSTIFICATIONHANDLER_H
