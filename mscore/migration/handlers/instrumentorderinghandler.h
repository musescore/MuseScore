#ifndef INSTRUMENTORDERINGMIGRATIONHANDLER_H
#define INSTRUMENTORDERINGMIGRATIONHANDLER_H

#include "migration/iscoremigrationhandler.h"

class InstrumentOrderingHandler : public IScoreMigrationHandler
      {
   public:
      InstrumentOrderingHandler() = default;

      bool handle(Ms::Score* score) override;
      };

#endif // INSTRUMENTORDERINGMIGRATIONHANDLER_H
