#ifndef MU_NOTATION_INSTRUMENTSCONVERTER_H
#define MU_NOTATION_INSTRUMENTSCONVERTER_H

#include "instruments/instrumentstypes.h"

namespace Ms {
class Instrument;
class NamedEventList;
}

namespace mu::notation {
class InstrumentsConveter
{
public:
    static Ms::Instrument convertInstrument(const instruments::Instrument& instrument);
    static instruments::Instrument convertInstrument(const Ms::Instrument& insturment);

private:
    static instruments::MidiActionList convertMidiActions(const QList<Ms::NamedEventList>& midiActions);
    static QList<Ms::NamedEventList> convertMidiActions(const instruments::MidiActionList& midiActions);
};
}

#endif // MU_NOTATION_INSTRUMENTSCONVERTER_H
