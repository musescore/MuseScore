#include "automationdurationmult.h"

#include "libmscore/undo.h"

using namespace mu::pianoroll;

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

double AutomationDurationMult::value(Ms::Staff* staff, NoteEventBlock& block)
{
    Ms::Note* note = block.note;
    Ms::NoteEvent* evt = &(note->playEvents()[0]);
    return evt->len();
}

void AutomationDurationMult::setValue(Ms::Staff* staff, NoteEventBlock& block, double value)
{
    Ms::Note* note = block.note;
    Ms::NoteEvent* evt = &(note->playEvents()[0]);
    Ms::Score* score = staff->score();

    Ms::NoteEvent ne = *evt;
    ne.setLen(value);

    score->startCmd();
    score->undo(new Ms::ChangeNoteEvent(note, evt, ne));
    score->endCmd();
}
