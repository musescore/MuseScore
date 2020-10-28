#include "firstsystemindentationhandler.h"

bool FirstSystemIndentationHandler::handle(Ms::Score* score)
      {
      if (!score)
            return false;

      score->undoChangeStyleVal(Ms::Sid::enableIndentationOnFirstSystem, true);

      return true;
      }
