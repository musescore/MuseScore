#include "resetallelementspositionshandler.h"

bool ResetAllElementsPositionsHandler::handle(Ms::Score* score)
{
    if (!score) {
        return false;
    }

    score->resetAllPositions();

    return true;
}
