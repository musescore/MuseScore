#include "staffverticaljustificationhandler.h"

bool StaffVerticalJustificationHandler::handle(Ms::Score* score)
      {
      if (!score) {
            return false;
      }
      score->undoChangeStyleVal(Ms::Sid::enableVerticalSpread, true);

      return true;
      }
