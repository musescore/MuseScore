#include "edwinstylehandler.h"

bool EdwinStyleHandler::handle(Ms::Score* score)
      {
      if (!score)
            return false;

      return score->loadStyle(":/styles/Edwin.mss", /*ign*/false, /*overlap*/true);
      }
