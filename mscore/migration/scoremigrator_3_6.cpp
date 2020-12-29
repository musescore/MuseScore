#include "scoremigrator_3_6.h"

#include "libmscore/mscore.h"
#include "preferences.h"
#include "libmscore/undo.h"

#include "handlers/styledefaultshandler.h"
#include "handlers/lelandstylehandler.h"
#include "handlers/edwinstylehandler.h"
#include "handlers/resetallelementspositionshandler.h"

ScoreMigrator_3_6::~ScoreMigrator_3_6()
      {
      qDeleteAll(m_handlerList);
      }

void ScoreMigrator_3_6::registerHandler(IScoreMigrationHandler* handler)
      {
      m_handlerList << handler;
      }

void ScoreMigrator_3_6::migrateScore(Ms::Score* score)
      {
      if (!score)
            return;

      score->startCmd();

      bool successfulMigration = false;

      for (IScoreMigrationHandler* handler : qAsConst(m_handlerList)) {
            successfulMigration = handler->handle(score);

            if (!successfulMigration)
                  break;
            }

      if (successfulMigration && score->mscVersion() != Ms::MSCVERSION)
            score->undo(new Ms::ChangeMetaText(score, "mscVersion", MSC_VERSION));

      score->endCmd();
      }
