#ifndef SCOREMIGRATIONREGISTER_H
#define SCOREMIGRATIONREGISTER_H

#include <QList>

#include "libmscore/score.h"
#include "iscoremigrationhandler.h"

class ScoreMigrator_3_6
      {
   public:
      ScoreMigrator_3_6() = default;
      ~ScoreMigrator_3_6();

      void registerHandler(IScoreMigrationHandler* handler);
      void migrateScore(Ms::Score* score);

   private:
      QList<IScoreMigrationHandler*> m_handlerList;
      };

#endif // SCOREMIGRATIONREGISTER_H
