#include "lelandstylehandler.h"

bool LelandStyleHandler::handle(Ms::Score* score)
      {
      if (!score)
            return false;

      return score->loadStyle(":/styles/Leland.mss", /*ign*/false, /*overlap*/true);
      }
