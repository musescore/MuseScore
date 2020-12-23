#ifndef OLDSCOREHANDLER_H
#define OLDSCOREHANDLER_H

#include "migration/iscoremigrationhandler.h"

class StyleDefaultsHandler : public IScoreMigrationHandler
      {
    public:
      StyleDefaultsHandler() = default;

      bool handle(Ms::Score* score) override;
    private:
      int resolveDefaultsVersion(const int mscVersion) const;
      void applyStyleDefaults(Ms::Score* score) const;
      };

#endif // OLDSCOREHANDLER_H
