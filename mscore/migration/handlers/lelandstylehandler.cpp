#include "lelandstylehandler.h"

#include "libmscore/excerpt.h"
#include "libmscore/score.h"

bool LelandStyleHandler::handle(Ms::Score* score)
      {
      if (!score)
            return false;

      for (Ms::Excerpt* excerpt : score->excerpts()) {
            if (!excerpt->partScore()->loadStyle(":/styles/Leland.mss", /*ign*/false, /*overlap*/true))
                  return false;
            }

      return score->loadStyle(":/styles/Leland.mss", /*ign*/false, /*overlap*/true);
      }
