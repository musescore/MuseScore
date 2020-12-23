#include "edwinstylehandler.h"

#include "libmscore/excerpt.h"
#include "libmscore/score.h"

bool EdwinStyleHandler::handle(Ms::Score* score)
      {
      if (!score)
            return false;

      for (Ms::Excerpt* excerpt : score->excerpts()) {
            if (!excerpt->partScore()->loadStyle(":/styles/Edwin.mss", /*ign*/false, /*overlap*/true))
                  return false;
            }

      return score->loadStyle(":/styles/Edwin.mss", /*ign*/false, /*overlap*/true);
      }
