#ifndef FIRSTSYSTEMINDENTATIONHANDLER_H
#define FIRSTSYSTEMINDENTATIONHANDLER_H

#include "migration/iscoremigrationhandler.h"

class FirstSystemIndentationHandler : public IScoreMigrationHandler
      {
   public:
      FirstSystemIndentationHandler() = default;

      bool handle(Ms::Score* score) override;
      };

#endif // FIRSTSYSTEMINDENTATIONHANDLER_H
