#include "musescore.h"
#include "instrumentorderinghandler.h"

bool InstrumentOrderingHandler::handle(Ms::Score* /*score*/)
      {
      Ms::mscore->editInstrumentList();
      return true;
      }
