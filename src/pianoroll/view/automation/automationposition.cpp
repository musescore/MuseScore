#include "automationposition.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;
using namespace mu::engraving;

AutomationPosition::AutomationPosition()
{
}

PianorollAutomationNote::AutomationType AutomationPosition::type()
{
    return PianorollAutomationNote::AutomationType::POSITION;
}

double AutomationPosition::maxValue()
{
    return 1000;
}

double AutomationPosition::minValue()
{
    return -1000;
}

double AutomationPosition::value(Staff* staff, NoteEventBlock& block)
{
    Note* note = block.note;
    NoteEvent* evt = &(note->playEvents()[0]);

    return evt->ontime();
}

void AutomationPosition::setValue(Staff* staff, NoteEventBlock& block, double value)
{
    Note* note = block.note;
    NoteEvent* evt = &(note->playEvents()[0]);
    Score* score = staff->score();

    NoteEvent ne = *evt;
    ne.setOntime(value);

    score->startCmd();
    score->undo(new ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}
