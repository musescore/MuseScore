#ifndef MU_NOTATION_INOTATIONMIDIDATA_H
#define MU_NOTATION_INOTATIONMIDIDATA_H

#include <memory>

#include "retval.h"
#include "midi/miditypes.h"

#include "notation/notationtypes.h"
#include "notation/internal/igetscore.h"

namespace mu::notation {
class INotationMidiEvents
{
public:
    virtual ~INotationMidiEvents() = default;

    virtual void init() = 0;
    virtual midi::Events retrieveEvents(const midi::channel_t midiChannel, const midi::tick_t fromTick,
                                        const midi::tick_t toTick) const = 0;
    virtual midi::Events retrieveEventsForElement(const Element* element, const midi::channel_t midiChannel) const = 0;
    virtual std::vector<midi::Event> retrieveSetupEvents(const std::list<InstrumentChannel*> instrChannel) const = 0;
};

using INotationMidiEventsPtr = std::shared_ptr<INotationMidiEvents>;
}

#endif // MU_NOTATION_INOTATIONMIDIDATA_H
