#ifndef EDWINSTYLEHANDLER_H
#define EDWINSTYLEHANDLER_H

#include "migration/iscoremigrationhandler.h"

class EdwinStyleHandler : public IScoreMigrationHandler
      {
    public:
      EdwinStyleHandler() = default;

      bool handle(Ms::Score* score) override;
      };

#endif // EDWINSTYLEHANDLER_H
