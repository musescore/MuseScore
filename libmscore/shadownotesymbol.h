#ifndef __SHADOWNOTESYMBOL_H__
#define __SHADOWNOTESYMBOL_H__

#include "duration.h"
#include "durationtype.h"
#include "sym.h"

namespace Ms {

class TDuration;

//---------------------------------------------------------
//    @@ NoteSymbol
///     This class implements a note symbol which can be used for placing notes
//    @P isFullMeasure  bool  (read only)
//---------------------------------------------------------

class ShadowNoteSymbol {


    public:
    ShadowNoteSymbol():
        symNotehead(SymId::noSym),
        symFlag(SymId::noSym),
        bNoteSymSet(false),
        bFlagSymSet(false)
    {

    }

   public:
      ~ShadowNoteSymbol();
      void setSymbols(TDuration::DurationType type, SymId noteSymbol);
      SymId getNoteId() const { return symNotehead; }
      SymId getFlagId() const { return symFlag; }
      void clearSymbols();
      bool isValid() const { return bNoteSymSet; }
      bool noFlag() const { return bFlagSymSet ? false:true ; }

  private:
      SymId symNotehead;
      SymId symFlag;
      bool bNoteSymSet;
      bool bFlagSymSet;


      };

}     // namespace Ms

#endif // __SHADOWNOTESYMBOL_H__

