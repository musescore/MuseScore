#include "automationdurationmult.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;
using namespace mu::engraving;

AutomationDurationMult::AutomationDurationMult()
{
}

PianorollAutomationNote::AutomationType AutomationDurationMult::type()
{
    return PianorollAutomationNote::AutomationType::DURATION_MULT;
}

double AutomationDurationMult::maxValue()
{
    return 1000;
}

double AutomationDurationMult::minValue()
{
    return 0;
}

double AutomationDurationMult::value(Staff* staff, NoteEventBlock& block)
{
    Note* note = block.note;
    NoteEvent* evt = &(note->playEvents()[0]);
    return evt->len();
}

void AutomationDurationMult::setValue(Staff* staff, NoteEventBlock& block, double value)
{
    Note* note = block.note;
    NoteEvent* evt = &(note->playEvents()[0]);
    Score* score = staff->score();

    NoteEvent ne = *evt;
    ne.setLen(value);

    score->startCmd();
    score->undo(new ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}
